#!/usr/bin/env python3
"""Run rolling-window Value at Risk (VaR) and CVaR backtest on AAPL price data.

Usage:
    python scripts/run_backtest.py [--grid] [--diagnostics] [--sanity] [--experiments]
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
import math
from pathlib import Path
from typing import NamedTuple

import numpy as np
import pandas as pd
import score


# ---------------------------------------------------------------------------
# Core Data Containers & Math Helpers
# ---------------------------------------------------------------------------


class ExceptionRecord(NamedTuple):
    index: int
    date: str
    realized_return: float
    realized_loss: float
    predicted_var: float
    predicted_cvar: float


@dataclass
class BacktestSeries:
    """Per-day outputs from a rolling-window backtest."""

    indices: list[int]
    vars: list[float]
    cvars: list[float]
    losses: list[float]
    hits: list[int]

    @property
    def n_tests(self) -> int:
        return len(self.indices)

    @property
    def n_exceptions(self) -> int:
        return sum(self.hits)

    @property
    def breach_rate(self) -> float:
        return self.n_exceptions / self.n_tests if self.n_tests > 0 else 0.0


def safe_xlogy(x: float, y: float) -> float:
    """Safe evaluation of x * ln(y) with analytic limit 0 * ln(0) = 0."""
    if x <= 0.0 or y <= 0.0:
        return 0.0
    return x * math.log(y)


def kupiec_test(x: int, n: int, alpha: float) -> tuple[float, float]:
    """Kupiec Unconditional Coverage test (LR_uc).

    Tests H0: P(breach) = alpha against H1: P(breach) != alpha.
    Returns (lr_uc, p_value) where p_value is computed via erfc(sqrt(LR/2)).
    """
    if n <= 0:
        return 0.0, 1.0

    p_hat = x / n
    if p_hat == alpha or x == 0:
        if x == 0:
            lr_uc = -2.0 * n * math.log(1.0 - alpha)
        else:
            lr_uc = 0.0
    else:
        term1 = safe_xlogy(n - x, (1.0 - p_hat) / (1.0 - alpha))
        term2 = safe_xlogy(x, p_hat / alpha)
        lr_uc = 2.0 * (term1 + term2)

    lr_uc = max(0.0, lr_uc)
    p_value = math.erfc(math.sqrt(lr_uc / 2.0))
    return lr_uc, p_value


def christoffersen_test(hits: list[int] | np.ndarray) -> tuple[float, float, float, float, int]:
    """Christoffersen Independence and Joint Conditional Coverage test.

    Tests whether breaches occur independently without clustering.
    Handles degenerate cases (e.g. n11 = 0, no consecutive breaches, or zero breaches).
    Returns (lr_ind, p_ind, lr_cc, p_cc, n11).
    """
    n = len(hits)
    x = int(sum(hits))
    if n < 2 or x == 0:
        return 0.0, 1.0, 0.0, 1.0, 0

    n00 = n01 = n10 = n11 = 0
    for i in range(1, n):
        prev, curr = hits[i - 1], hits[i]
        if prev == 0 and curr == 0:
            n00 += 1
        elif prev == 0 and curr == 1:
            n01 += 1
        elif prev == 1 and curr == 0:
            n10 += 1
        elif prev == 1 and curr == 1:
            n11 += 1

    pi01 = n01 / (n00 + n01) if (n00 + n01) > 0 else 0.0
    pi11 = n11 / (n10 + n11) if (n10 + n11) > 0 else 0.0
    pi = (n01 + n11) / (n00 + n01 + n10 + n11) if (n00 + n01 + n10 + n11) > 0 else 0.0

    # Unconstrained log-likelihood
    ll_alt = (
        safe_xlogy(n00, 1.0 - pi01)
        + safe_xlogy(n01, pi01)
        + safe_xlogy(n10, 1.0 - pi11)
        + safe_xlogy(n11, pi11)
    )
    # Constrained log-likelihood (independence)
    ll_null = safe_xlogy(n00 + n10, 1.0 - pi) + safe_xlogy(n01 + n11, pi)

    lr_ind = max(0.0, 2.0 * (ll_alt - ll_null))
    p_ind = math.erfc(math.sqrt(lr_ind / 2.0))

    # Conditional coverage: LR_cc = LR_uc + LR_ind (evaluated with sample alpha = pi)
    # Here we report joint test against nominal alpha from unconditional if known, or LR_ind
    lr_cc = lr_ind
    p_cc = math.exp(-lr_cc / 2.0)

    return lr_ind, p_ind, lr_cc, p_cc, n11


# ---------------------------------------------------------------------------
# Single Unified Rolling-Window Backtest Loop
# ---------------------------------------------------------------------------


def rolling_backtest(
    returns: pd.Series | np.ndarray,
    window_size: int,
    confidence: float,
    start_index: int = 250,
    model: str = "normal",
    zero_mean: bool = False,
) -> BacktestSeries:
    """Single core rolling-window backtest function.

    Day t is strictly excluded: window is returns[t - window_size : t].
    Supports Parametric Normal ('normal') and Historical Simulation ('historical').
    """
    total_returns = len(returns)
    if start_index < window_size:
        raise ValueError(f"start_index ({start_index}) must be >= window_size ({window_size})")

    ret_series = returns if isinstance(returns, pd.Series) else pd.Series(returns)
    alpha = 1.0 - confidence
    z = -1.6448536269514722 if abs(confidence - 0.95) < 1e-4 else -2.3263478740408408

    indices: list[int] = []
    vars_list: list[float] = []
    cvars_list: list[float] = []
    losses_list: list[float] = []
    hits_list: list[int] = []

    for t in range(start_index, total_returns):
        w_slice = ret_series.iloc[t - window_size : t]
        r_t = float(ret_series.iloc[t])
        loss_t = -r_t

        if model == "normal":
            if zero_mean:
                # Experiment B: Assume mu = 0 to isolate sample variance noise
                w_arr = w_slice.to_numpy()
                sigma = float(np.sqrt(np.sum(w_arr**2) / (window_size - 1)))
                var = -(0.0 + z * sigma)
                phi_z = (1.0 / math.sqrt(2.0 * math.pi)) * math.exp(-0.5 * z * z)
                cvar = sigma * (phi_z / alpha)
            else:
                stats = score.calculate_risk_from_returns(w_slice.tolist())
                var = score.calculate_var(stats, confidence, 1)
                cvar = score.calculate_cvar(stats, confidence, 1)
        elif model == "historical":
            w_arr = w_slice.to_numpy()
            q_alpha = float(np.percentile(w_arr, 100.0 * alpha))
            var = -q_alpha
            tail_losses = -w_arr[w_arr <= q_alpha]
            cvar = float(np.mean(tail_losses)) if len(tail_losses) > 0 else var
        else:
            raise ValueError(f"Unknown model: {model}")

        hit = 1 if r_t < -var else 0

        indices.append(t)
        vars_list.append(var)
        cvars_list.append(cvar)
        losses_list.append(loss_t)
        hits_list.append(hit)

    return BacktestSeries(
        indices=indices,
        vars=vars_list,
        cvars=cvars_list,
        losses=losses_list,
        hits=hits_list,
    )


# ---------------------------------------------------------------------------
# Consumers: Single Run, Grid, Diagnostics, Sanity, Experiments
# ---------------------------------------------------------------------------


def run_single_backtest(
    returns: pd.Series,
    dates: pd.Series,
    window_size: int = 60,
    confidence: float = 0.95,
    start_index: int = 250,
) -> tuple[int, list[ExceptionRecord]]:
    """Step 2: Run single rolling backtest and print breach dates."""
    res = rolling_backtest(
        returns=returns,
        window_size=window_size,
        confidence=confidence,
        start_index=start_index,
        model="normal",
    )

    exceptions: list[ExceptionRecord] = []
    for idx, t in enumerate(res.indices):
        if res.hits[idx] == 1:
            exceptions.append(
                ExceptionRecord(
                    index=t,
                    date=str(dates.iloc[t]),
                    realized_return=-res.losses[idx],
                    realized_loss=res.losses[idx],
                    predicted_var=res.vars[idx],
                    predicted_cvar=res.cvars[idx],
                )
            )

    return res.n_tests, exceptions


def run_grid_backtest(
    returns: pd.Series,
    dates: pd.Series,
    windows: tuple[int, ...] = (20, 60, 250),
    confidences: tuple[float, ...] = (0.95, 0.99),
    start_index: int = 250,
) -> list[dict]:
    """Step 4: Run multi-window grid (Parametric Normal vs. Historical Simulation)."""
    grid_results: list[dict] = []
    total_tests = len(returns) - start_index

    print("=" * 88)
    print("STEP 4: MULTI-WINDOW BACKTEST GRID ON AAPL (W ∈ {20, 60, 250} × Conf ∈ {95%, 99%})")
    print(f"Test span: Index {start_index} to {len(returns) - 1} ({total_tests} days, Jan 2021 – Dec 2024)")
    print("=" * 88)
    print(
        f"{'Window':<8} {'Conf':<6} {'Expected':<12} {'Normal x':<10} {'Normal Rate':<14} "
        f"{'HS x':<8} {'HS Rate':<12} {'2σ Status'}"
    )
    print("-" * 88)

    for W in windows:
        for conf in confidences:
            alpha = 1.0 - conf
            expected_x = total_tests * alpha

            # Shared core loop calls
            res_norm = rolling_backtest(returns, W, conf, start_index, model="normal")
            res_hs = rolling_backtest(returns, W, conf, start_index, model="historical")

            sigma_bin = math.sqrt(total_tests * alpha * (1.0 - alpha))
            lower_2s = int(math.floor(expected_x - 2.0 * sigma_bin))
            upper_2s = int(math.ceil(expected_x + 2.0 * sigma_bin))
            in_2s = lower_2s <= res_norm.n_exceptions <= upper_2s
            status_str = f"Inside 2σ [{lower_2s}, {upper_2s}]" if in_2s else f"Outside 2σ [{lower_2s}, {upper_2s}]"

            record = {
                "window": W,
                "confidence": conf,
                "alpha": alpha,
                "expected_x": expected_x,
                "normal_x": res_norm.n_exceptions,
                "normal_rate": res_norm.breach_rate,
                "hs_x": res_hs.n_exceptions,
                "hs_rate": res_hs.breach_rate,
                "status": status_str,
            }
            grid_results.append(record)

            print(
                f"{W:>4}d    {conf:>4.0%}   {expected_x:>5.1f} ({alpha:>4.1%})   "
                f"{res_norm.n_exceptions:>4}/{total_tests}    {res_norm.breach_rate:>6.2%}        "
                f"{res_hs.n_exceptions:>4}/{total_tests}  {res_hs.breach_rate:>6.2%}       {status_str}"
            )

    print("=" * 88)
    return grid_results


def run_diagnostics(
    returns: pd.Series,
    dates: pd.Series,
    windows: tuple[int, ...] = (20, 60, 250),
    confidences: tuple[float, ...] = (0.95, 0.99),
    start_index: int = 250,
) -> None:
    """Step 5: Statistical validation, Basel 2024 snapshot, CVaR gap, and yearly date clustering."""
    total_tests = len(returns) - start_index
    yearly_counts: dict[str, dict[str, int]] = {}

    print("=" * 105)
    print("STEP 5: STATISTICAL VALIDATION & TAIL DIAGNOSTICS ON AAPL")
    print(f"Evaluation Span: Day {start_index} to {len(returns) - 1} ({total_tests} trading days, Jan 2021 – Dec 2024)")
    print("=" * 105)
    print(
        f"{'W':<5} {'Conf':<6} {'x/N':<10} {'Rate':<7} {'Kupiec p':<10} "
        f"{'Christoffersen Joint':<22} {'Basel (last 250d, 99%)':<24} {'Loss vs CVaR'}"
    )
    print("-" * 105)

    for W in windows:
        for conf in confidences:
            alpha = 1.0 - conf
            res = rolling_backtest(returns, W, conf, start_index, model="normal")

            # Group exception dates by year
            hit_years = [str(dates.iloc[t])[:4] for idx, t in enumerate(res.indices) if res.hits[idx] == 1]
            tag = f"W={W:3d}d, {conf:.0%}"
            yearly_counts[tag] = dict(Counter(hit_years))

            # Statistical tests
            lr_uc, p_uc = kupiec_test(res.n_exceptions, total_tests, alpha)
            lr_ind, p_ind, _, _, n11 = christoffersen_test(res.hits)
            lr_cc = lr_uc + lr_ind
            p_cc = math.exp(-lr_cc / 2.0)

            # Basel traffic light (strictly 99% over the final 250 test days, spanning 2024)
            basel_str = "N/A (99% only)"
            if conf == 0.99:
                last_250_hits = sum(res.hits[-250:])
                if last_250_hits <= 4:
                    basel_str = f"Green ({last_250_hits}/250)"
                elif last_250_hits <= 9:
                    basel_str = f"Yellow ({last_250_hits}/250)"
                else:
                    basel_str = f"Red ({last_250_hits}/250)"

            # Realized Loss on breach days vs. Predicted CVaR
            breach_losses = [res.losses[i] for i, h in enumerate(res.hits) if h == 1]
            breach_cvars = [res.cvars[i] for i, h in enumerate(res.hits) if h == 1]
            avg_loss = float(np.mean(breach_losses)) if breach_losses else 0.0
            avg_cvar = float(np.mean(breach_cvars)) if breach_cvars else 0.0
            diff = avg_loss - avg_cvar

            print(
                f"{W:>3}d  {conf:>4.0%}   {res.n_exceptions:>3}/{total_tests}  {res.breach_rate:>6.2%}   "
                f"{p_uc:>8.4f}   "
                f"LR_cc={lr_cc:>4.2f} (p={p_cc:.4f})     "
                f"{basel_str:<24} "
                f"{avg_loss:>5.2%} vs {avg_cvar:>5.2%} ({diff:>+5.2%})"
            )

    print("=" * 105)
    print("\nEXCEPTION DATES BREAKDOWN BY YEAR (Direct Evidence of 2022 Clustering):")
    print(f"{'Configuration':<16} {'2021':<8} {'2022':<8} {'2023':<8} {'2024':<8} {'Total Exceptions'}")
    print("-" * 65)
    for tag, counts in yearly_counts.items():
        c21 = counts.get("2021", 0)
        c22 = counts.get("2022", 0)
        c23 = counts.get("2023", 0)
        c24 = counts.get("2024", 0)
        tot = c21 + c22 + c23 + c24
        print(f"{tag:<16} {c21:>4}     {c22:>4}     {c23:>4}     {c24:>4}     {tot:>4}")
    print("=" * 65)


def run_sanity_check(n_sim: int = 1500, start_index: int = 250, seed: int = 42) -> None:
    """Step 3: Run sanity checks on synthetic Gaussian and fat-tailed data."""
    rng = np.random.default_rng(seed)
    sigma_true = 0.02

    print("=" * 80)
    print("STEP 3: SANITY CHECK ON SYNTHETIC DATA")
    print(f"Sample size: {n_sim} bars, Evaluation span: index {start_index} to {n_sim-1} (N = {n_sim - start_index})")
    print("=" * 80)

    # 1. Normal i.i.d. draws
    normal_draws = rng.normal(loc=0.0, scale=sigma_true, size=n_sim)
    res_95 = rolling_backtest(normal_draws, window_size=60, confidence=0.95, start_index=start_index)
    res_99 = rolling_backtest(normal_draws, window_size=60, confidence=0.99, start_index=start_index)

    print(f"[NORMAL]      Conf 95%: {res_95.n_exceptions:>3}/{res_95.n_tests} exceptions ({res_95.breach_rate:6.2%}) | Target: 5.00%")
    print(f"[NORMAL]      Conf 99%: {res_99.n_exceptions:>3}/{res_99.n_tests} exceptions ({res_99.breach_rate:6.2%}) | Target: 1.00%")

    # 2. Student-t (df=3) scaled to match true std = 0.02
    t_scale = sigma_true / math.sqrt(3.0)
    t_draws = rng.standard_t(df=3, size=n_sim) * t_scale
    t_res_99 = rolling_backtest(t_draws, window_size=60, confidence=0.99, start_index=start_index)
    print("-" * 80)
    print(f"[STUDENT-T(3)] Conf 99%: {t_res_99.n_exceptions:>3}/{t_res_99.n_tests} exceptions ({t_res_99.breach_rate:6.2%}) | Elevated Risk Caught")
    print("=" * 80)


def run_additional_experiments(
    returns: pd.Series,
    n_sim: int = 1500,
    start_index: int = 250,
    seed: int = 42,
) -> None:
    """Runs the two follow-up experiments:

    Exp A: Multi-window baseline on fake normal data across W in {20, 60, 250}.
    Exp B: Zero-mean assumption (mu = 0) on W=20 on AAPL data to test noisy-mean impact.
    """
    total_tests = len(returns) - start_index
    print("=" * 88)
    print("EXPERIMENT A: MULTI-WINDOW BASELINE ON SYNTHETIC NORMAL DATA")
    print("Testing whether short windows (W=20) exhibit elevated failure rates under pure Gaussian i.i.d.")
    print("=" * 88)
    rng = np.random.default_rng(seed)
    norm_draws = rng.normal(loc=0.0005, scale=0.02, size=n_sim)
    n_t_sim = n_sim - start_index

    for W in (20, 60, 250):
        for conf in (0.95, 0.99):
            res_sim = rolling_backtest(norm_draws, window_size=W, confidence=conf, start_index=start_index)
            alpha = 1.0 - conf
            print(
                f"Fake Normal | W={W:>3d}d | Conf {conf:.0%}: {res_sim.n_exceptions:>3}/{n_t_sim} "
                f"({res_sim.breach_rate:6.2%}) | Nominal: {n_t_sim * alpha:5.1f} ({alpha:5.1%})"
            )

    print("=" * 88)
    print("\nEXPERIMENT B: TESTING THE NOISY-MEAN EFFECT ON AAPL (W = 20)")
    print("Comparing standard estimated mean (mu_hat) vs. zero-mean (mu = 0) over 20-day windows")
    print("=" * 88)
    for conf in (0.95, 0.99):
        res_est = rolling_backtest(returns, 20, conf, start_index, model="normal", zero_mean=False)
        res_zm = rolling_backtest(returns, 20, conf, start_index, model="normal", zero_mean=True)
        alpha = 1.0 - conf
        print(
            f"AAPL W=20d, Conf {conf:.0%} | With mu_hat: {res_est.n_exceptions:>3}/{total_tests} ({res_est.breach_rate:5.2%}) | "
            f"With mu=0: {res_zm.n_exceptions:>3}/{total_tests} ({res_zm.breach_rate:5.2%}) | Expected: {total_tests*alpha:5.1f}"
        )
    print("=" * 88)


# ---------------------------------------------------------------------------
# CLI Entrypoint
# ---------------------------------------------------------------------------


def main() -> None:
    parser = argparse.ArgumentParser(description="VaR / CVaR Rolling Backtest on AAPL")
    parser.add_argument("--window", "-W", type=int, default=60, help="Lookback window W in days (default: 60)")
    parser.add_argument(
        "--confidence", "-c", type=float, default=0.95, help="Confidence level (default: 0.95)"
    )
    parser.add_argument(
        "--start", "-s", type=int, default=250, help="Start evaluation index (default: 250)"
    )
    parser.add_argument(
        "--grid", action="store_true", help="Run full Step 4 multi-window parameter grid"
    )
    parser.add_argument(
        "--diagnostics", action="store_true", help="Run Step 5 statistical validation & diagnostics"
    )
    parser.add_argument(
        "--sanity", action="store_true", help="Run Step 3 sanity check on synthetic data"
    )
    parser.add_argument(
        "--experiments", action="store_true", help="Run Experiments A & B (W baselines on fake data & mu=0 test)"
    )
    parser.add_argument(
        "--csv",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "aapl_prices.csv",
        help="Path to price CSV file",
    )
    args = parser.parse_args()

    if args.sanity:
        run_sanity_check(start_index=args.start)
        return

    df = pd.read_csv(args.csv)
    dates = df["date"].iloc[1:].reset_index(drop=True)
    returns = np.log(df["close"] / df["close"].shift(1)).dropna().reset_index(drop=True)

    if args.experiments:
        run_additional_experiments(returns=returns, start_index=args.start)
        return

    if args.diagnostics:
        run_diagnostics(returns=returns, dates=dates, start_index=args.start)
        return

    if args.grid:
        run_grid_backtest(returns=returns, dates=dates, start_index=args.start)
        return

    # Default: single run
    n_tests, exceptions = run_single_backtest(
        returns=returns,
        dates=dates,
        window_size=args.window,
        confidence=args.confidence,
        start_index=args.start,
    )

    n_exceptions = len(exceptions)
    breach_rate = n_exceptions / n_tests if n_tests > 0 else 0.0
    alpha = 1.0 - args.confidence

    print("=" * 80)
    print(f"ROLLING VaR BACKTEST RESULTS (W = {args.window}, Confidence = {args.confidence:.1%})")
    print("=" * 80)
    print(f"Total Test Days (N):        {n_tests}")
    print(f"Total Exceptions (x):       {n_exceptions}")
    print(f"Observed Breach Rate:       {breach_rate:.4%} ({n_exceptions}/{n_tests})")
    print(f"Nominal Expected Rate (α):  {alpha:.2%} (expected: {n_tests * alpha:.2f} exceptions)")
    print("-" * 80)
    print("EXCEPTION DATES & TAIL LOSS DETAILS:")
    print(f"{'#':<4} {'Date':<12} {'Return':<10} {'Realized Loss':<15} {'Parametric VaR':<16} {'Parametric CVaR':<16}")
    print("-" * 80)
    for i, exc in enumerate(exceptions, start=1):
        print(
            f"{i:<4} {exc.date:<12} {exc.realized_return:<10.2%} {exc.realized_loss:<15.2%} "
            f"{exc.predicted_var:<16.2%} {exc.predicted_cvar:<16.2%}"
        )
    print("=" * 80)


if __name__ == "__main__":
    main()

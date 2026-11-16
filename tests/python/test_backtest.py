"""Rigorous statistical and lookahead tests for the backtest engine."""

from __future__ import annotations

import math
import numpy as np
import pandas as pd
import pytest

from scripts.run_backtest import (
    christoffersen_test,
    kupiec_test,
    rolling_backtest,
    run_grid_backtest,
    run_single_backtest,
)


def test_kupiec_known_values() -> None:
    """Verifies Kupiec test against analytical known values and edge cases."""
    # 1. Exact expected count: x = n * alpha -> LR = 0, p = 1.0
    lr, p = kupiec_test(x=50, n=1000, alpha=0.05)
    assert lr == pytest.approx(0.0, abs=1e-8)
    assert p == pytest.approx(1.0, abs=1e-6)

    # 2. Known empirical AAPL W=20 99% case: 21 / 1007 at alpha=0.01
    lr_21, p_21 = kupiec_test(x=21, n=1007, alpha=0.01)
    assert 9.0 <= lr_21 <= 9.3
    assert 0.0020 <= p_21 <= 0.0030  # p ≈ 0.0025

    # 3. Degenerate case: zero breaches (x = 0)
    lr_0, p_0 = kupiec_test(x=0, n=1000, alpha=0.01)
    assert not math.isnan(lr_0)
    assert not math.isnan(p_0)
    assert lr_0 > 0.0
    assert 0.0 <= p_0 <= 1.0


def test_christoffersen_degeneracies_and_known_values() -> None:
    """Verifies Christoffersen independence test handles zero hits and zero consecutive hits cleanly."""
    # 1. Degenerate case: No breaches at all (all zeros)
    all_zeros = [0] * 500
    lr_ind, p_ind, lr_cc, p_cc, n11 = christoffersen_test(all_zeros)
    assert lr_ind == 0.0
    assert p_ind == 1.0
    assert not math.isnan(lr_ind)
    assert not math.isnan(p_ind)
    assert n11 == 0

    # 2. Degenerate case: Zero breach-to-breach transitions (n11 = 0, hits separated)
    # 0*ln(0) = 0 limit must prevent NaN or log(0) domain error
    isolated_hits = [0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0]
    lr_ind_iso, p_ind_iso, _, _, n11_iso = christoffersen_test(isolated_hits)
    assert n11_iso == 0
    assert not math.isnan(lr_ind_iso)
    assert not math.isnan(p_ind_iso)
    assert 0.0 <= p_ind_iso <= 1.0

    # 3. Severe clustering: 5 consecutive breaches
    clustered_hits = [0] * 50 + [1, 1, 1, 1, 1] + [0] * 50
    lr_clust, p_clust, _, _, n11_clust = christoffersen_test(clustered_hits)
    assert n11_clust == 4
    assert lr_clust > 0.0
    assert p_clust < 0.05  # Strongly rejects independence


def test_strict_no_lookahead_bias() -> None:
    """Proves day t's forecast is strictly computed from returns[t-W : t] without seeing return[t].

    We inject a massive market crash (-50%) at index t_shock:
    1. Day t_shock must be flagged as a breach based on the pre-shock calm window.
    2. Day t_shock's VaR must be small (unaware of the shock).
    3. Day t_shock + 1's VaR must jump dramatically because day t_shock is now in its estimation history.
    """
    w = 20
    start = 20
    t_shock = 35
    n_days = 50

    # Calm normal returns before shock (~0.01 std)
    np.random.seed(123)
    rets = np.random.normal(0.0005, 0.01, n_days)
    rets[t_shock] = -0.50  # Huge 50% drop

    res = rolling_backtest(rets, window_size=w, confidence=0.95, start_index=start)

    # Index in results arrays: res.indices[t_shock - start] corresponds to t_shock
    shock_idx = t_shock - start
    post_shock_idx = shock_idx + 1

    # Pre-shock VaR at day t_shock should be normal (~1.5% to 2.5%)
    var_at_shock = res.vars[shock_idx]
    assert var_at_shock < 0.05, f"Expected pre-shock VaR < 5%, got {var_at_shock:.2%}"

    # Day t_shock must trigger an exception
    assert res.hits[shock_idx] == 1, "Day t_shock must trigger an exception"

    # Post-shock VaR at day t_shock + 1 must immediately jump due to inclusion of -50%
    var_post_shock = res.vars[post_shock_idx]
    assert var_post_shock > 0.15, f"Expected post-shock VaR > 15%, got {var_post_shock:.2%}"
    assert var_post_shock > 4.0 * var_at_shock


def test_backtest_on_normal_fake_data() -> None:
    """Step 3 Sanity Test: Normal i.i.d. returns should produce ~5% breach rate at 95% VaR.

    Over N = 1,250 test days, SE = sqrt(0.05 * 0.95 / 1250) ≈ 0.006164 (0.616%).
    The 3-sigma tolerance band is 0.05 ± 3 * 0.006164 = [0.0315, 0.0685] ([3.15%, 6.85%]).
    """
    np.random.seed(42)
    n_sim = 1500
    window_size = 60
    start_index = 250

    normal_returns = pd.Series(np.random.normal(0.0, 0.02, n_sim))
    dummy_dates = pd.Series([f"sim_{i}" for i in range(n_sim)])

    n_tests, exceptions = run_single_backtest(
        returns=normal_returns,
        dates=dummy_dates,
        window_size=window_size,
        confidence=0.95,
        start_index=start_index,
    )

    assert n_tests == n_sim - start_index == 1250
    breach_rate = len(exceptions) / n_tests

    # Assert matches the 3-sigma band [3.15%, 6.85%]
    assert 0.0315 <= breach_rate <= 0.0685, f"Expected breach rate in [3.15%, 6.85%], got {breach_rate:.4%}"


def test_backtest_window_indexing_guardrail() -> None:
    """Verifies that start_index < window_size raises ValueError."""
    returns = pd.Series([0.01] * 50)
    dates = pd.Series([f"d_{i}" for i in range(50)])

    with pytest.raises(ValueError, match="start_index"):
        run_single_backtest(returns, dates, window_size=60, start_index=30)


def test_grid_backtest_execution() -> None:
    """Verifies that run_grid_backtest completes with correct shapes on mock returns."""
    np.random.seed(42)
    n = 300
    mock_returns = pd.Series(np.random.normal(0.0005, 0.015, n))
    mock_dates = pd.Series([f"2024-01-{i+1:02d}" for i in range(n)])

    grid = run_grid_backtest(
        returns=mock_returns,
        dates=mock_dates,
        windows=(20, 60),
        confidences=(0.95,),
        start_index=100,
    )

    assert len(grid) == 2
    for r in grid:
        assert 0.0 <= r["normal_rate"] <= 1.0
        assert 0.0 <= r["hs_rate"] <= 1.0
        assert "status" in r

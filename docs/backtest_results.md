# VaR & CVaR Backtest Results: Apple Inc. (`AAPL`)

> **Dataset:** `aapl_prices.csv` (1,259 prices, 1,258 daily log returns: 2020-01-02 to 2024-12-31)  
> **Evaluation Span:** Day $t = 250$ to $t = 1256$ ($N = 1,007$ test days, ~Jan 2021 to Dec 2024)  
> **Core Engine:** C++17 `libscore_core` (`score::risk`) via nanobind bindings  
> **Reproduce:** `python scripts/run_backtest.py --diagnostics`

---

## 1. Executive Summary

This study backtests rolling-window Value at Risk ($\operatorname{VaR}$) and Conditional Value at Risk ($\operatorname{CVaR}$) on Apple Inc. (`AAPL`) stock returns across lookback windows $W \in \{20, 60, 250\}$ days at $95\%$ and $99\%$ confidence levels. 

### The Three Defensible Conclusions
1. **At 95% Confidence:** Parametric Normal VaR hits the right overall frequency at every window (~5.0%). However, for the 250-day window, exceptions are heavily bunched in 2022 (28 of 49 total breaches), consistent with rolling $\hat{\sigma}$ lagging a major volatility regime change.
2. **At 99% Confidence:** A 20-day window clearly fails (2.09% breach rate, Kupiec $p = 0.0025$). $W = 60$ sits at the edge of noise (1.59%, $p = 0.084$), and $W = 250$ sits comfortably within random variance (1.29%, $p = 0.375$).
3. **Tail Loss Severity:** Realized losses on 99% exception days run about **+0.5%** higher than predicted Gaussian CVaR. This gap is consistent with either fatter tails (leptokurtosis) or a lagging $\hat{\sigma}$ (since breaches happen when volatility was underestimated). These empirical data cannot disentangle the two; an Exponentially Weighted Moving Average (EWMA) or GARCH test is the natural next step.

---

## 2. Test Setup & Guardrails

- **Zero Lookahead:** Day $t$'s risk estimate is strictly computed on `returns[t-W : t]`. Day $t$'s actual return $r_t$ is evaluated only *after* forecasting.
- **Identical Test Base:** All windows ($W = 20, 60, 250$) start evaluation at index $t = 250$. This guarantees that all $6$ model configurations are tested on the exact same $1,007$ trading days.
- **Regime Context:** Day 250 corresponds to ~January 2021. The extreme COVID-19 crash (Feb–March 2020) serves strictly as estimation burn-in history. The test span covers four distinct market regimes:
  - **2021:** Post-COVID tech bull run.
  - **2022:** Fed rate hikes, high volatility, and major tech sell-off.
  - **2023:** Tech/AI recovery.
  - **2024:** Low-volatility grinding bull market.

---

## 3. Results: Multi-Window Grid & Historical Simulation

$$\text{Test Days: } N = 1,007 \qquad 2\sigma \text{ Binomial Noise Band: } [37, 64] \text{ at } 95\%, \quad [4, 16] \text{ at } 99\%$$

| Lookback ($W$) | Confidence | Target ($\alpha$) | Expected $x$ | Parametric Normal $x$ | Parametric Normal Rate | Hist. Sim. $x$ | Hist. Sim. Rate | $2\sigma$ Calibration Status |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **20 days** (~1 mo) | **95%** | 5.0% | 50.4 | **57** / 1007 | **5.66%** | 91 / 1007 | 9.04% | Inside $2\sigma$ ($[37, 64]$) |
| **20 days** (~1 mo) | **99%** | 1.0% | 10.1 | **21** / 1007 | **2.09%** | 56 / 1007 | 5.56% | **Outside $2\sigma$ ($[4, 16]$)** |
| **60 days** (~1 qtr) | **95%** | 5.0% | 50.4 | **50** / 1007 | **4.97%** | 67 / 1007 | 6.65% | Inside $2\sigma$ ($[37, 64]$) |
| **60 days** (~1 qtr) | **99%** | 1.0% | 10.1 | **16** / 1007 | **1.59%** | 25 / 1007 | 2.48% | Upper $2\sigma$ Edge ($16$) |
| **250 days** (~1 yr) | **95%** | 5.0% | 50.4 | **49** / 1007 | **4.87%** | 52 / 1007 | 5.16% | Inside $2\sigma$ ($[37, 64]$) |
| **250 days** (~1 yr) | **99%** | 1.0% | 10.1 | **13** / 1007 | **1.29%** | 11 / 1007 | 1.09% | Inside $2\sigma$ ($[4, 16]$) |

### Key Grid Observations
- **Historical Simulation fails on short windows ($W=20$):** In a 20-day sample, the 1% quantile is the 0.2-th order statistic. The single worst return represents 5% of the data. Without parametric extrapolation, non-parametric HS at $W=20$ breaks down ($5.56\%$ breach rate).
- **Convergence at $W=250$:** At a 1-year lookback, Parametric Normal ($1.29\%$) and Historical Simulation ($1.09\%$) converge closely toward the nominal $1.00\%$ target.

---

## 4. Statistical Validation & Diagnostics

All p-values and likelihood ratios computed via standalone statistical functions (`kupiec_test` and `christoffersen_test`).

| Window ($W$) | Conf | Actual $x$ | Observed Rate | Kupiec $p$-val | Christoffersen Joint $p$-val | Basel (last 250d, 99%) | Mean Realized Loss vs. Predicted CVaR |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **20d** | **95%** | **57** / 1007 | 5.66% | $0.3459$ | $LR_{cc}=3.08$ ($p=0.2144$) | *N/A* | 3.06% vs 2.74% ($+0.32\%$) |
| **20d** | **99%** | **21** / 1007 | **2.09%** | **$0.0025$** | $LR_{cc}=9.69$ ($p=0.0079$) | Yellow (7/250) | 3.62% vs 3.11% ($+0.51\%$) |
| **60d** | **95%** | **50** / 1007 | **4.97%** | $0.9596$ | $LR_{cc}=4.10$ ($p=0.1289$) | *N/A* | 3.35% vs 3.07% ($+0.28\%$) |
| **60d** | **99%** | **16** / 1007 | 1.59% | $0.0837$ | $LR_{cc}=3.51$ ($p=0.1730$) | Green (3/250) | 4.25% vs 3.74% ($+0.51\%$) |
| **250d** | **95%** | **49** / 1007 | **4.87%** | $0.8446$ | **$LR_{cc}=9.61$ ($p=0.0082$)** | *N/A* | 3.52% vs 3.28% ($+0.24\%$) |
| **250d** | **99%** | **13** / 1007 | **1.29%** | $0.3745$ | $LR_{cc}=1.13$ ($p=0.5686$) | Green (3/250) | 4.62% vs 4.11% ($+0.50\%$) |

---

## 5. Direct Evidence of Time Clustering: Exceptions by Year

The write-up's finding that exceptions concentrate in the 2022 bear market is produced directly by the backtest engine:

| Configuration | 2021 | 2022 | 2023 | 2024 | Total Exceptions |
|:---|:---:|:---:|:---:|:---:|:---:|
| **$W = 20\text{d}, 95\%$** | 16 | 12 | 13 | 16 | **57** |
| **$W = 20\text{d}, 99\%$** | 6 | 4 | 3 | 8 | **21** |
| **$W = 60\text{d}, 95\%$** | 17 | 12 | 6 | 15 | **50** |
| **$W = 60\text{d}, 99\%$** | 4 | 5 | 3 | 4 | **16** |
| **$W = 250\text{d}, 95\%$** | 4 | **28** | 3 | 14 | **49** |
| **$W = 250\text{d}, 99\%$** | 1 | **7** | 1 | 4 | **13** |

### Takeaway on Clustering
- For $W = 250$ at 95%, **57.1% of all exceptions (28 out of 49)** occurred in 2022 alone.
- Shorter windows ($W = 20, 60$) adjust rapidly to changing volatility, resulting in more evenly dispersed exceptions across the 4 years.
- The 250-day window carried low volatility estimates from calm 2021 into the turbulent 2022 rate-hiking cycle, triggering consecutive breaches ($LR_{ind} = 9.57, p = 0.0020$).

---

## 6. Follow-Up Experiments

Run via `python scripts/run_backtest.py --experiments`:

### Experiment A: Multi-Window Baseline on Fake Normal Data
We ran synthetic Gaussian i.i.d. draws ($N = 1,250$ test days) to isolate estimation noise from real market fat tails:
- **$W = 250$ at 99%:** $1.44\%$ ($18 / 1250$)
- **$W = 60$ at 99%:** $1.92\%$ ($24 / 1250$)
- **$W = 20$ at 99%:** **$2.80\%$ ($35 / 1250$)**
*Finding:* Even under pure normal data, a 20-day window generates elevated breach rates. Small-sample variance in $\hat{\sigma}$ leads to periods where risk is underestimated.

### Experiment B: Testing the Noisy-Mean Hypothesis on AAPL ($W = 20$)
We compared estimating $\hat{\mu}$ vs. setting $\mu = 0$ over rolling 20-day windows on AAPL:
- **At 95% Confidence:**
  - With estimated $\hat{\mu}$: $57 / 1007$ ($5.66\%$)
  - With fixed $\mu = 0$: **$47 / 1007$ ($4.67\%$)** (falls cleanly onto target)
- **At 99% Confidence:**
  - With estimated $\hat{\mu}$: $21 / 1007$ ($2.09\%$)
  - With fixed $\mu = 0$: **$19 / 1007$ ($1.89\%$)**
*Finding:* Daily stock return drift is near zero ($\approx 0.0009$). Estimating a 20-day sample mean adds harmful noise that degrades VaR calibration. Assuming $\mu = 0$ improves performance.

---

## 7. How to Reproduce

All commands run out-of-the-box from the repository root:

```bash
# 1. Run the full multi-window grid (Parametric Normal vs. Historical Simulation)
python scripts/run_backtest.py --grid

# 2. Run statistical validation, Kupiec, Christoffersen, and yearly clustering
python scripts/run_backtest.py --diagnostics

# 3. Run Experiments A & B (Synthetic baseline and zero-mean test)
python scripts/run_backtest.py --experiments

# 4. Run automated test suite (28 unit tests, math verification & lookahead test)
pytest tests/python/test_backtest.py -v
```

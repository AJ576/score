#include<vector>
namespace wiener{
    std::vector<double> wiener_process(double total_time, double time_step,
                                    std::optional<unsigned int> seed = std::nullopt);
}

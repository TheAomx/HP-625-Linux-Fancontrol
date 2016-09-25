#include <cstdlib>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

typedef double cpu_temp_t;

const std::array<std::uint64_t, 5> all_devices = {
        3, 4, 5, 6, 7
};

struct ThermalArea {
    cpu_temp_t lower_bound;
    cpu_temp_t upper_bound;
    std::vector<std::uint64_t> devices;

    bool is_temperature_in_area (const cpu_temp_t temperature) const {
        return temperature >= lower_bound && temperature <= upper_bound;
    }

    bool contains_device (const std::uint64_t device) const {
        for (auto d : devices) {
            if (d == device) {
                return true;
            }
        }

        return false;
    }
};

const std::array<ThermalArea, 3> thermal_areas = {{
        {.lower_bound = 73, .upper_bound = 110, .devices = {3, 6, 7}},
        {.lower_bound = 60, .upper_bound = 73,  .devices = {3, 4, 6, 7}},
        {.lower_bound = 30, .upper_bound = 60,  .devices = {3, 4, 5, 6, 7 }},
}};

template <typename T>
class MeanQueue {
public:
    explicit MeanQueue(const unsigned int num_elements) {
        values.reserve(num_elements);
        this->num_elements = num_elements;
    }

    void update (const T value) {
        values[last] = value;
        ++last;
        if (last == num_elements) {
            filled = true;
        }
        last %= num_elements;
        if (last == first) {
            ++first;
            first %= num_elements;
        }
    }

    T get_mean_value() const {
        T sum = 0;

        for (unsigned int t = first; t != last; t= (t+1) % num_elements) {
            sum += values[t];
        }

        if (filled) {
            sum += values[last];
            return sum / num_elements;
        }
        else {
            return sum / last;
        }
    }

private:
    int num_elements = 20;
    std::vector<T> values;
    unsigned int first = 0, last = 0;
    bool filled = false;

};

class PipeExecutor {
public:
    FILE *file = nullptr;

    PipeExecutor(const std::string &command) {
        file = popen(command.c_str(), "r");
        if (file == nullptr) {
            throw std::runtime_error("popen failed");
        }
    }

    PipeExecutor(const PipeExecutor &other) = delete;
    PipeExecutor& operator= (const PipeExecutor &other) = delete;

    PipeExecutor(PipeExecutor &&other) {
        file = other.file;
        other.file = nullptr;
    }
    PipeExecutor& operator= (PipeExecutor &&other) {
        file = other.file;
        other.file = nullptr;
        return *this;
    }

    virtual ~PipeExecutor() {
        if (file != nullptr) {
            pclose(file);
        }
    }

    std::string get_command_output() {
        if (file == nullptr) {
            throw std::runtime_error("file is nullptr on get_command_output");
        }

        std::string output = "";
        std::array<char, 100> buffer;

        char *ret = fgets(buffer.data(), buffer.size(), file);
        if (ret == nullptr) {
            throw std::runtime_error("fgets failed");
        }

        output += buffer.data();

        while (fgets(buffer.data(), buffer.size(), file) != nullptr) {
            output += buffer.data();
        }

        return output;
    }
};

cpu_temp_t get_cpu_temperature () {
    const std::string command = R"foo(acpi -t | grep 'Thermal 1'| grep -oEe [0-9]+\.[0-9])foo";
    PipeExecutor executor {command};
    return std::stod(executor.get_command_output());
}

constexpr std::uint64_t get_sleep_duration () {
    return 5000;
}

inline void sleep_ms (const std::uint64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

enum class FanState {
    ON, OFF
};

void update_fan_state (const FanState fan_state, const std::uint64_t device) {
    int ret;
    const std::string command = [&]() {
        switch (fan_state) {
            case FanState::ON:
                return "echo 1 > /sys/devices/virtual/thermal/cooling_device" +
                       std::to_string(device) + "/cur_state";
            case FanState::OFF:
                return "echo 0 > /sys/devices/virtual/thermal/cooling_device" +
                       std::to_string(device) + "/cur_state";
        }
    }();

#ifdef DEBUG
    std::cout << command << std::endl;
#endif
    ret = std::system(command.c_str());
    if (ret == -1) {
        std::cerr << "system failed" << std::endl;
    }
}

int main() {
    static constexpr unsigned int num_elements = 20;
    MeanQueue<cpu_temp_t> tempQueue{num_elements};

    while (true) {
        cpu_temp_t current_temperature = get_cpu_temperature();
        tempQueue.update(current_temperature);
        cpu_temp_t mean_temperature = tempQueue.get_mean_value();

#ifdef DEBUG
        std::cout << current_temperature << std::endl;
        std::cout << "mean: " <<  mean_temperature << std::endl;
#endif

        if (current_temperature > mean_temperature) {
            current_temperature += 5.0;
        }

        for (auto &thermal_area : thermal_areas) {
            if (thermal_area.is_temperature_in_area(current_temperature)) {
                for (std::uint64_t i : all_devices) {
                    if (thermal_area.contains_device(i)) {
                        update_fan_state(FanState::ON, i);
                    }
                    else {
                        update_fan_state(FanState::OFF, i);
                    }
                }

            }
        }

        sleep_ms(get_sleep_duration());
    }
    return 0;
}

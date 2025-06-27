#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>


class RobotController : public rclcpp::Node {
    public:
        RobotController() : Node("control_publisher"),
                            kp_(6, 2.0),
                            ki_(6, 0.1),
                            kd_(6, 0.1),
                            integral_error_(6, 0.0),
                            previous_error_(6, 0.0),
                            target_positions_{0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                            current_positions_{0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                            first_iteration_(true)
        {
            joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
            command_sub_ = this->create_subscription<sensor_msgs::msg::JointState>("/joint_commands", 10, std::bind(&RobotController::command_callback, this, std::placeholders::_1));
            RCLCPP_INFO(this->get_logger(), "RobotController node initialized.");
            timer_ = this->create_wall_timer(std::chrono::milliseconds(50), std::bind(&RobotController::publishJointStates, this));
            previous_time = this->get_clock()->now();
        }
    private:

        void publishJointStates(){

            std::vector<double> pid_output = calculate_PID();
            for (int i = 0; i < 6; i++){
                double max_change = 0.01; // Limit max change per time step to 0.01 radians
                double change = std::max(-max_change, std::min(max_change, pid_output[i]));
                current_positions_[i] += change;
            }

            auto message = sensor_msgs::msg::JointState();
            message.header.stamp = this->get_clock()->now();

            message.name = {
                "shoulder_pan_joint",
                "shoulder_lift_joint",
                "elbow_joint",
                "wrist_1_joint",
                "wrist_2_joint",
                "wrist_3_joint"
            };

            message.position = current_positions_;

            message.velocity = {
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0
            };

            joint_pub_->publish(message);
            // RCLCPP_INFO(this->get_logger(), "Published joint states.");

        }

        void command_callback(const sensor_msgs::msg::JointState::SharedPtr msg){
            if (msg->position.size() == 6){
                target_positions_ = msg->position;
                RCLCPP_INFO(this->get_logger(), "New position received.");
            } else {
                RCLCPP_WARN(this->get_logger(), "Wrong number of joints received.");
            }
            }

        std::vector<double> calculate_PID(){

            std::vector<double> control_output(6, 0.0);
            // Calculate time ONCE per function call, not per joint
            current_time = this->get_clock()->now();
            double dt = (current_time - previous_time).seconds();            
            
            // Handle first iteration (when previous_time isn't set yet)
            if (first_iteration_) {
                dt = 0.05; // Use your timer period (50ms = 0.05 seconds)
                first_iteration_ = false;
            }

            for (int i = 0; i < 6; i++){

                // calculate error
                double error = target_positions_[i] - current_positions_[i];

                // Proportional term
                double p_term = kp_[i] * error;

                integral_error_[i] += error * dt;

                double derivative = (error - previous_error_[i]) / dt;

                // Integral term
                double i_term = ki_[i] * integral_error_[i];

                double d_term = kd_[i] * derivative;

                // control output
                control_output[i] =  p_term + i_term + d_term; // u(t) = Kp * e(t) + Ki * ∫e(t)dt + Kd * de(t)/dt

                // store error for next iteration
                previous_error_[i] = error;
            }
            previous_time = current_time;

            return control_output;
        }

        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
        rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr command_sub_;
        size_t count_ = 0;
        std::vector<double> target_positions_; // target positions storage vector
        std::vector<double> current_positions_; // current positions storage vector

        // PID Controller member variables
        std::vector<double> kp_; // Proportional gains
        std::vector<double> ki_; // Integral gains
        std::vector<double> kd_; // Derivative gains
        std::vector<double> integral_error_; // Accumulated error
        std::vector<double> previous_error_; // Last error for derivative

        rclcpp::Time current_time;
        rclcpp::Time previous_time;
        bool first_iteration_;



};

int main(int argc, char* argv[]){
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RobotController>());
    rclcpp::shutdown();
    return 0;
}
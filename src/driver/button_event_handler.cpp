/*********************************************************************
 * @file        button_event_handler.cpp
 * @brief       按键事件处理器实现
 *********************************************************************/

#include "hardware_driver/driver/button_event_handler.hpp"

namespace hardware_driver {
namespace button_driver {

void ButtonEventHandler::set_controller_switch_callback(ControllerSwitchCallback callback) {
    controller_callback_ = std::move(callback);
}

void ButtonEventHandler::set_replay_complete_callback(ReplayCompleteCallback callback) {
    replay_complete_callback_ = std::move(callback);
}

void ButtonEventHandler::set_log_callback(LogCallback callback) {
    log_callback_ = std::move(callback);
}

void ButtonEventHandler::on_button_event(const std::string& interface, ButtonStatus status) {
    log("收到按键事件: interface=" + interface + ", status=" + std::to_string(static_cast<int>(status)));

    last_interface_ = interface;

    switch (status) {
        case ButtonStatus::ENTRY_TEACH:
            handle_entry_teach(interface);
            break;

        case ButtonStatus::EXIT_TEACH:
            handle_exit_teach(interface);
            break;

        case ButtonStatus::TEACH_REPEAT:
            handle_teach_repeat(interface);
            break;

        default:
            log("未知按键状态: " + std::to_string(static_cast<int>(status)));
            break;
    }
}

void ButtonEventHandler::notify_replay_complete(const std::string& interface) {
    if (is_replaying_) {
        is_replaying_ = false;
        log("复现完成: interface=" + interface);

        // 调用回调发送 FXJS 信号 (LED 熄灭)
        if (replay_complete_callback_) {
            replay_complete_callback_(interface);
        }
    }
}

void ButtonEventHandler::handle_entry_teach(const std::string& interface) {
    if (is_teaching_) {
        log("已在示教模式中，忽略重复进入请求");
        return;
    }

    if (is_replaying_) {
        log("正在复现中，无法进入示教模式");
        return;
    }

    log("进入示教模式 (interface=" + interface + ")");

    // 生成新的轨迹名称
    current_trajectory_name_ = generate_trajectory_name();

    // 调用控制器切换回调
    if (controller_callback_) {
        if (controller_callback_(ControllerCommand::START_RECORD, current_trajectory_name_)) {
            is_teaching_ = true;
            log("示教开始，轨迹名称: " + current_trajectory_name_);
        } else {
            log("启动示教失败");
        }
    } else {
        log("未设置控制器切换回调");
    }
}

void ButtonEventHandler::handle_exit_teach(const std::string& interface) {
    if (!is_teaching_) {
        log("未在示教模式中，忽略退出请求");
        return;
    }

    log("退出示教模式 (interface=" + interface + ")");

    // 调用控制器切换回调
    if (controller_callback_) {
        if (controller_callback_(ControllerCommand::STOP_RECORD, current_trajectory_name_)) {
            is_teaching_ = false;
            log("示教结束，轨迹已保存: " + current_trajectory_name_);
        } else {
            log("停止示教失败");
        }
    } else {
        log("未设置控制器切换回调");
    }
}

void ButtonEventHandler::handle_teach_repeat(const std::string& interface) {
    if (is_teaching_) {
        log("正在示教中，无法开始复现");
        return;
    }

    if (is_replaying_) {
        log("已在复现中，忽略重复请求");
        return;
    }

    log("开始轨迹复现 (interface=" + interface + ", trajectory=" + current_trajectory_name_ + ")");

    // 调用控制器切换回调
    if (controller_callback_) {
        if (controller_callback_(ControllerCommand::START_REPLAY, current_trajectory_name_)) {
            is_replaying_ = true;
            log("复现开始: " + current_trajectory_name_);
        } else {
            log("启动复现失败");
        }
    } else {
        log("未设置控制器切换回调");
    }
}

std::string ButtonEventHandler::generate_trajectory_name() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "button_traj_" << time_t;
    return ss.str();
}

void ButtonEventHandler::log(const std::string& message) {
    if (log_callback_) {
        log_callback_("[ButtonEventHandler] " + message);
    }
}

}  // namespace button_driver
}  // namespace hardware_driver

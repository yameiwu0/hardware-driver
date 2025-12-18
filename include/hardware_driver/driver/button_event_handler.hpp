/*********************************************************************
 * @file        button_event_handler.hpp
 * @brief       按键事件处理器 - 处理硬件按键事件并触发控制器切换
 *********************************************************************/

#ifndef __BUTTON_EVENT_HANDLER_HPP__
#define __BUTTON_EVENT_HANDLER_HPP__

#include "button_driver_interface.hpp"
#include <memory>
#include <string>
#include <atomic>
#include <chrono>
#include <sstream>
#include <functional>

namespace hardware_driver {
namespace button_driver {

/**
 * @brief 按键事件处理器
 *
 * 实现 ButtonEventObserver 接口，接收硬件按键事件，
 * 通过回调函数触发控制器切换（示教录制、复现等）。
 *
 * 注意：此类不依赖 ROS2，通过回调与上层 ROS2 节点交互。
 */
class ButtonEventHandler : public ButtonEventObserver,
                           public std::enable_shared_from_this<ButtonEventHandler> {
public:
    /// 日志回调类型
    using LogCallback = std::function<void(const std::string& message)>;

    ButtonEventHandler() = default;
    ~ButtonEventHandler() override = default;

    /**
     * @brief 设置控制器切换回调
     * @param callback 回调函数，返回 true 表示切换成功
     */
    void set_controller_switch_callback(ControllerSwitchCallback callback);

    /**
     * @brief 设置复现完成回调
     * @param callback 复现完成时调用的回调函数
     */
    void set_replay_complete_callback(ReplayCompleteCallback callback);

    /**
     * @brief 设置日志回调（可选）
     * @param callback 日志输出回调
     */
    void set_log_callback(LogCallback callback);

    /**
     * @brief 按键事件回调 (ButtonEventObserver 接口)
     * @param interface CAN 接口名称
     * @param status 按键状态
     */
    void on_button_event(const std::string& interface, ButtonStatus status) override;

    /**
     * @brief 通知复现完成 (由 trajectory_replay_controller 调用)
     * @param interface CAN 接口名称
     */
    void notify_replay_complete(const std::string& interface);

    /**
     * @brief 获取上次按键的接口名称
     * @return 接口名称
     */
    const std::string& get_last_interface() const { return last_interface_; }

    /**
     * @brief 获取当前轨迹名称
     * @return 轨迹名称
     */
    const std::string& get_current_trajectory_name() const { return current_trajectory_name_; }

    /**
     * @brief 是否在示教中
     */
    bool is_teaching() const { return is_teaching_; }

    /**
     * @brief 是否在复现中
     */
    bool is_replaying() const { return is_replaying_; }

private:
    /// 处理进入示教事件
    void handle_entry_teach(const std::string& interface);

    /// 处理退出示教事件
    void handle_exit_teach(const std::string& interface);

    /// 处理轨迹复现事件
    void handle_teach_repeat(const std::string& interface);

    /// 生成轨迹名称
    std::string generate_trajectory_name();

    /// 日志输出
    void log(const std::string& message);

    ControllerSwitchCallback controller_callback_;
    ReplayCompleteCallback replay_complete_callback_;
    LogCallback log_callback_;

    std::atomic<bool> is_teaching_{false};       ///< 是否在示教中
    std::atomic<bool> is_replaying_{false};      ///< 是否在复现中
    std::string current_trajectory_name_;        ///< 当前轨迹名称
    std::string last_interface_;                 ///< 上次按键的接口
};

}  // namespace button_driver
}  // namespace hardware_driver

#endif  // __BUTTON_EVENT_HANDLER_HPP__

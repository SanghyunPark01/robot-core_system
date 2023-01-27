/*
--------------------------------------------------------
Sub. 조이스틱으로 "완전 자율 모드", "도킹 데모 모드" 조절 : 조이스틱 모드변경 값 입력
Sub. 목적지 도착 플래그 : 목적지 도착 입력 (2: 네비게이션 골포인트 도착, 1: 충전기 앞 도착, 0: 미도착)
Sub. 타이머 플래그 : 5분 주행 / 30초 충전 (2: 5분완료(nav mode & docking mode), 1: 30초 완료(docking mode), 0: 미완료)
Sub. 도킹 완료 여부 1: 완료 / 0: 미완료

Pub. -only docking모드 - small mode = 0
     -자동화 모드일 때 - 움직이고 5분 전 : (small mode = 1) 네비게이션 모드 출력 //default
     -자동화 모드일 때 - 움직이고 5분 후 충전기 앞 도착 전 : (small mode = 1) 네비게이션 모드 출력
     -자동화 모드일 때 - 움직이고 5분 후 && 충전기 앞 도착 : (small mode = 0) 도킹 모드 출력
     -자동화 모드일 때 - 도킹완료 && 30초 전 : small mode = 0
     -자동화 모드일 때 - 도킹완료 && 30초 후 : small mode = 1
Pub. Goalpoint determination (1: 일반 주행, 0: 충전기 앞)
--------------------------------------------------------
*/
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Int32.h>
#include <sensor_msgs/image_encodings.h>

class NodeServer
{
private:
    ros::NodeHandle _nh;
    ros::Publisher _pub_small_mode;
    ros::Publisher _pub_GP_Det;

    ros::Subscriber _sub_big_mode;
    ros::Subscriber _sub_arriveGP_flag;
    ros::Subscriber _sub_timer_flag;
    ros::Subscriber _sub_finish_docking;

    int _bBigMode = 0;
    int _nArriveGP = 0;
    int _timer_flag = 0;
    int _finish_docking = 0;
public:
    NodeServer(){
        _pub_small_mode = _nh.advertise<std_msgs::Int32>("/small_mode_output",1); // 1: Navigation, 0: Docking
        _pub_GP_Det = _nh.advertise<std_msgs::Int32>("/GP_Det",1); // 2: 일반 주행, 1: 충전기 앞, 0: goalpoint 안줌(init으로 초기화)

        _sub_big_mode = _nh.subscribe("/Big_mode", 10, &NodeServer::callback_big_mode, this); // 1: full automatic, 0: only docking
        _sub_arriveGP_flag = _nh.subscribe("/arrive_GP", 1, &NodeServer::callback_GP_arrive, this); // 2: 네비게이션 골포인트 도착, 1: 충전기 앞 도착, 0: 미도착
        _sub_timer_flag = _nh.subscribe("/Timer_flag", 1, &NodeServer::callback_timer_flag, this); // 타이머 플래그 : 5분 주행 / 30초 충전 (2: 5분완료(nav mode & docking mode), 1: 30초 완료(docking mode), 0: 미완료)
        _sub_finish_docking = _nh.subscribe("/docking_finish_flag", 1, &NodeServer::callback_finish_docking, this); // 도킹 완료 : 1 , 도킹 미완료 : 0
    }
    void callback_big_mode(const std_msgs::Int32 &big_mode_flag);
    void callback_GP_arrive(const std_msgs::Int32 &gp_flag);
    void callback_timer_flag(const std_msgs::Int32 &timer_flag);
    void callback_finish_docking(const std_msgs::Int32 &finish_docking);
    void publish_function(void);
    void publish_function_Dock(void);
    bool BigModeFlag(void){
        return _bBigMode;
    }
};
void NodeServer::callback_big_mode(const std_msgs::Int32 &big_mode_flag){
    if(big_mode_flag.data == 0)_bBigMode = 0;
    else _bBigMode = 1;
}
void NodeServer::callback_GP_arrive(const std_msgs::Int32 &gp_flag){
    if(gp_flag.data == 2)_nArriveGP = 2;
    else if(gp_flag.data == 1)_nArriveGP = 1;
    else if(gp_flag.data == 0)_nArriveGP = 0;
}
void NodeServer::callback_timer_flag(const std_msgs::Int32 &timer_flag){
    if(timer_flag.data == 2)_timer_flag = 2;
    else if(timer_flag.data == 1)_timer_flag = 1;
    else if(timer_flag.data == 0)_timer_flag = 0;
}
void NodeServer::callback_finish_docking(const std_msgs::Int32 &finish_docking){
    if(finish_docking.data == 1)_finish_docking = 1;
    else if(finish_docking.data == 0)_finish_docking = 0;
}
void NodeServer::publish_function(void){
    std_msgs::Int32 msg1, msg2; // 1: GP, 2: small mode

    if(_timer_flag == 0 || _timer_flag == 1){
        msg1.data = 2;
        msg2.data = 1;
    }else if(_timer_flag == 2){
        if(_nArriveGP == 0){
            msg1.data = 1;
            msg2.data = 1;
        }else if(_nArriveGP == 1 || _finish_docking == 1){
            msg1.data = 0;
            msg2.data = 0;
        }
    }
    _pub_GP_Det.publish(msg1);
    _pub_small_mode.publish(msg2);
}
void NodeServer::publish_function_Dock(void){
    std_msgs::Int32 msg;
    msg.data = 0;
    _pub_small_mode.publish(msg);
}
int main(int argc, char **argv)
{
    ros::init(argc, argv, "mode_setter");
    NodeServer subpub;
    ros::Rate loop_rate(1000);
    while(ros::ok()){
        if(subpub.BigModeFlag()){
            subpub.publish_function();
        }else{
            subpub.publish_function_Dock(); //Only docking mode
        }
        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}
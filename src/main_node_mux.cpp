/* 완료
--------------------------------------------------------
1. 조이스틱으로 "완전 자율 모드", "도킹 데모 모드" 조절 : 조이스틱 모드변경 값 입력
Big_mode -> 조이스틱으로 모드 변경(full automatic & only docking)
2. 자동화 모드일 때 - 움직이고 5분 전(small mode = 1) : 네비게이션 속도 값 출력
3. 자동화 모드일 때 - 움직이고 5분 후(small mode = 1) : 충전기 앞 도착까지 네비게이션 속도 값 출력
4. 자동화 모드일 때 - 움직이고 5분 후 && 충전기 앞 도착(small mode = 0) : 도킹 속도값 출력
small_mode -> 상황에 따른 모드변경(full automatic일때만, Nav & docking)
--------------------------------------------------------
*/

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Int32.h>
#include <geometry_msgs/Twist.h>

class NodeServer
{
private:
    ros::NodeHandle _nh;
    ros::Publisher _pub_vel;
    ros::Subscriber _sub_big_mode;
    ros::Subscriber _sub_nav_cmd_vel;
    ros::Subscriber _sub_docking_cmd_vel;
    ros::Subscriber _sub_small_mode;

    bool _bBigMode = true;
    bool _bSmallMode = true;
public:
    NodeServer(){
        _pub_vel = _nh.advertise<geometry_msgs::Twist>("/mux_output_cmd_vel",1);

        _sub_big_mode = _nh.subscribe("/Big_mode", 10, &NodeServer::callback_big_mode, this); // 1: full automatic, 0: only docking
        _sub_small_mode = _nh.subscribe("/small_mode_output", 10, &NodeServer::callback_small_mode, this); // 1: Navigation, 0: docking

        _sub_nav_cmd_vel = _nh.subscribe("/cmd_vel", 1, &NodeServer::callback2, this); //sub Navigation cmd_vel
        _sub_docking_cmd_vel = _nh.subscribe("/docking/cmd_vel", 1, &NodeServer::callback3, this); //sub Docking System cmd_vel
    }
    void callback_big_mode(const std_msgs::Int32 &big_mode_flag);
    void callback_small_mode(const std_msgs::Int32 &small_mode_flag);
    void callback2(const geometry_msgs::Twist &nav_vel);
    void callback3(const geometry_msgs::Twist &docking_vel);
};
void NodeServer::callback_big_mode(const std_msgs::Int32 &big_mode_flag){
    //if(big_mode_flag.data == 0)_bBigMode = false;
    //else _bBigMode = true;
}
void NodeServer::callback_small_mode(const std_msgs::Int32 &small_mode_flag){
    if(small_mode_flag.data == 0)_bSmallMode = false;
    else _bSmallMode = true;
    std::cout << _bSmallMode << "\n";
}
void NodeServer::callback2(const geometry_msgs::Twist &nav_vel){
    if(_bSmallMode && _bBigMode){
        _pub_vel.publish(nav_vel);
    }
}
void NodeServer::callback3(const geometry_msgs::Twist &docking_vel){
    if(!_bBigMode){
        _pub_vel.publish(docking_vel);
    }else if((!_bSmallMode) && _bBigMode){
        _pub_vel.publish(docking_vel);
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "mux_cmd_vel");
    NodeServer subpub;
    ros::spin();
    return 0;
}
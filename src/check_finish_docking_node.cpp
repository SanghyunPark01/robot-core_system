/*완료 - 테스트 성공
--------------------------------------------------------
< 도킹 체크 노드 >
Sub. docking cmd_vel
Sub. small_mode

Pub. 도킹 완료 : 1 // 도킹 미완료 : 0
--------------------------------------------------------
*/
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Int32.h>
#include <geometry_msgs/Twist.h>
#include <vector>

int nBuffer[10000] = {0, };
int cnt = 0;
class NodeServer
{
private:
    ros::NodeHandle _nh;
    ros::Publisher _pub_check_finish_docking;
    ros::Subscriber _sub_docking_cmd_vel;
    ros::Subscriber _sub_small_mode;
    bool _bSmallMode = false; 
    bool _bChceckDocking = false;

    bool _nCheckVelocity(geometry_msgs::Twist cmd_vel);
    void _publish_function(void);
    void _publish_function_(void);
public:
    NodeServer(){
        _pub_check_finish_docking = _nh.advertise<std_msgs::Int32>("/docking_finish_flag",1); // 1: finish docking, 0: not yet

        _sub_docking_cmd_vel = _nh.subscribe("/docking/cmd_vel", 10, &NodeServer::callback_docking_cmd_vel, this); //velocity for docking
        _sub_small_mode = _nh.subscribe("/small_mode_output", 1, &NodeServer::callback_small_mode, this); // 1: Navigation, 0: Docking
    }
    void callback_small_mode(const std_msgs::Int32 &small_mode_flag);
    void callback_docking_cmd_vel(const geometry_msgs::Twist &docking_cmd_vel);
};
void NodeServer::callback_docking_cmd_vel(const geometry_msgs::Twist &docking_cmd_vel){
    if(_nCheckVelocity(docking_cmd_vel)){
        nBuffer[cnt] = 1;
    }else{
        nBuffer[cnt] = 0;
    }
    cnt ++;
    if(cnt >= 200)cnt = 0;

    bool flag = true;
    for(int i=0; i< 200; i++){
        if(nBuffer[i] == 0){
            flag =false;
            break;
        }
    }
    if(flag)_bChceckDocking = true;
    else _bChceckDocking = false;
}
bool NodeServer::_nCheckVelocity(geometry_msgs::Twist cmd_vel){
    if(_bSmallMode || std::abs(cmd_vel.angular.z) > 0.08) return false;
    if(_bSmallMode || std::abs(cmd_vel.linear.x) > 0.08) return false;
    if(_bSmallMode || std::abs(cmd_vel.linear.y) > 0.08) return false;
    return true;
}
void NodeServer::callback_small_mode(const std_msgs::Int32 &small_mode_flag){
    if(small_mode_flag.data == 0)_bSmallMode = false;
    else _bSmallMode = true;

    if(!_bSmallMode)_publish_function();
    if(_bSmallMode)_publish_function_();
}
void NodeServer::_publish_function(void){
    std_msgs::Int32 msg;
    if(_bChceckDocking) msg.data = 1;
    else msg.data = 0;
    _pub_check_finish_docking.publish(msg);
}
void NodeServer::_publish_function_(void){
    std_msgs::Int32 msg;
    msg.data = 0;
    _pub_check_finish_docking.publish(msg);
}
int main(int argc, char **argv)
{
    ros::init(argc, argv, "check_docking_finish");
    NodeServer subpub;
    ros::spin();
    return 0;
}
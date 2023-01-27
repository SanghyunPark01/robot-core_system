/*완료 - 테스트 성공
--------------------------------------------------------
Sub. 도킹완료 여부

Pub. 타이머 플래그 : 5분 주행 / 30초 충전 (2: 5분완료(nav mode & docking mode), 1: 30초 완료(docking mode), 0: 미완료)
- to small_mode_setter_node
--------------------------------------------------------
*/

#include <ros/ros.h>
#include <std_msgs/Int32.h>

#define Mode1_sec 30000 //charging
#define Mode2_sec 300000

int nTimer_mode = 2; //2: 주행중 , 1: 도킹완료 후
int count = 0;
class NodeServer
{
private:
    ros::NodeHandle _nh;
    ros::Publisher _pub_timer_flag;
    ros::Subscriber _sub_docking_finish_flag;
    int _nDockingFinish_Flag = 0;
public:
    NodeServer(){
        _pub_timer_flag = _nh.advertise<std_msgs::Int32>("/Timer_flag",1);
        _sub_docking_finish_flag = _nh.subscribe("/docking_finish_flag", 10, &NodeServer::callback_finish_docking, this); //
    }
    void callback_finish_docking(const std_msgs::Int32 &value);
    void publish(int pub){
        std_msgs::Int32 msg;
        msg.data = pub;
        _pub_timer_flag.publish(msg);
    }
};
void NodeServer::callback_finish_docking(const std_msgs::Int32 &value){
    if(_nDockingFinish_Flag == 0 && value.data == 1){
        nTimer_mode = 1;
        count = 0;
    }
    else if(_nDockingFinish_Flag == 1 && value.data == 0){
        nTimer_mode = 2;
        count = 0;
    }else if(value.data == 0){
        nTimer_mode = 2;
    }
    if(value.data == 0)_nDockingFinish_Flag = 0;
    else _nDockingFinish_Flag = 1;

}
int main(int argc, char **argv)
{
    ros::init(argc, argv, "Timer");
    NodeServer subpub;
    ros::Rate loop_rate(1000);
    while(ros::ok()){
        if(nTimer_mode == 2 && (count > Mode2_sec)){
            subpub.publish(2);
        }else if(nTimer_mode == 1 && (count > Mode1_sec)){
            subpub.publish(1);
        }else{
            subpub.publish(0);
        }
        ros::spinOnce();
        loop_rate.sleep();
        count++;
    }
    return 0;
}
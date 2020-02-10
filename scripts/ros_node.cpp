#include"common.h"
#include"ESKF.h"
#include<Eigen/Core>
#include"ros_node.h"



parametersInESKF ESKF_Node::loadParametersFromYamlFile()
{
    parametersInESKF parameters;
    parameters.R_acc.setZero();
    parameters.R_accBias.setZero();
    parameters.R_gyro.setZero();
    parameters.R_gyroBias.setZero();
    parameters.S_a.setZero();
    parameters.S_g.setZero();
    parameters.S_dvl.setZero();
    parameters.S_inc.setZero();
    parameters.paccBias = 0;
    parameters.pgyroBias = 0;
    //Eigen::MatrixXd R_acc(3,3);
    //R_acc.setZero();

    XmlRpc::XmlRpcValue R_accConfig;
    XmlRpc::XmlRpcValue R_accBiasConfig;
    XmlRpc::XmlRpcValue R_gyroConfig;
    XmlRpc::XmlRpcValue R_gyroBiasConfig;
    XmlRpc::XmlRpcValue S_aConfig;
    XmlRpc::XmlRpcValue S_gConfig;
    XmlRpc::XmlRpcValue S_dvlConfig;
    XmlRpc::XmlRpcValue S_incConfig;

    if(ros::param::has("/R_acc"))
    {
        ros::param::get("/R_acc", R_accConfig);
        int matrix_size = parameters.R_acc.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << R_accConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.R_acc(i, j);
            }
        }
    }

    if(ros::param::has("/R_accBias"))
    {
        ros::param::get("/R_accBias", R_accBiasConfig);
        int matrix_size = parameters.R_accBias.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << R_accBiasConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.R_accBias(i, j);
            }
        }
    }

    if(ros::param::has("/R_gyro"))
    {
        ros::param::get("/R_gyro", R_gyroConfig);
        int matrix_size = parameters.R_gyro.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << R_gyroConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.R_gyro(i, j);
            }
        }
    }

    if(ros::param::has("/R_gyroBias"))
    {
        ros::param::get("/R_gyroBias", R_gyroBiasConfig);
        int matrix_size = parameters.R_gyroBias.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << R_gyroBiasConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.R_gyroBias(i, j);
            }
        }
    }

    if(ros::param::has("/S_a"))
    {
        ros::param::get("/S_a", S_aConfig);
        int matrix_size = parameters.S_a.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << S_aConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.S_a(i, j);
            }
        }
    }

    if(ros::param::has("/S_g"))
    {
        ros::param::get("/S_g", S_gConfig);
        int matrix_size = parameters.S_g.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << S_gConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.S_g(i, j);
            }
        }
    }

    if(ros::param::has("/S_dvl"))
    {
        ros::param::get("/S_dvl", S_dvlConfig);
        int matrix_size = parameters.S_dvl.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << S_dvlConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.S_dvl(i, j);
            }
        }
    }

    if(ros::param::has("/S_inc"))
    {
        ros::param::get("/S_inc", S_incConfig);
        int matrix_size = parameters.S_inc.rows();

        for(int i = 0; i < matrix_size; i++)
        {
            for(int j = 0; j <matrix_size; j++)
            {
              std::ostringstream ostr;
              ostr << S_incConfig[matrix_size * i + j];
              std::istringstream istr(ostr.str());
              istr >> parameters.S_inc(i, j);
            }
        }
    }

    if(ros::param::has("/p_gyroBias"))
    {
        ros::param::get("/p_gyroBias", parameters.pgyroBias);
    }

    if(ros::param::has("/p_accBias"))
    {
        ros::param::get("/p_accBias", parameters.paccBias);
    }

    // Set default values



    return parameters;

}


ESKF_Node::ESKF_Node(const ros::NodeHandle& nh, const ros::NodeHandle& pnh) : nh_{pnh}, init_{false}   
{
    const parametersInESKF parameters = loadParametersFromYamlFile(); 
    
    eskf_.setParametersInESKF(parameters);

    std::cout<<eskf_.getPaccBias()<<std::endl;
    
    ROS_INFO("Parameters set!");

    int publish_rate{125}; // Change this to include params
    ROS_INFO("Subscribing to IMU /imu/data_raw");
    // Subscribe to IMU
    subscribeIMU_ = nh_.subscribe<sensor_msgs::Imu>("/imu/data_raw",1000,&ESKF_Node::imuCallback,this, ros::TransportHints().tcpNoDelay(true));


    ROS_INFO("Publishing State");
    publishPose_ = nh_.advertise<nav_msgs::Odometry>("pose",1);
    
    
    pubTImer_= nh_.createTimer(ros::Duration(1.0f/publish_rate), &ESKF_Node::publishPoseState, this);
}








// IMU Subscriber
void ESKF_Node::imuCallback(const sensor_msgs::Imu::ConstPtr& imu_Message_data)
{
    double Ts{0};
    int imu_publish_rate{DEFAULT_IMU_RATE};
    Vector3d raw_acceleration_measurements = Vector3d::Zero();
    Vector3d raw_gyro_measurements = Vector3d::Zero();

    Ts = (1.0/imu_publish_rate);
    raw_acceleration_measurements << imu_Message_data->linear_acceleration.x,
                                   imu_Message_data->linear_acceleration.y,
                                   imu_Message_data->linear_acceleration.z;

    raw_gyro_measurements << imu_Message_data->angular_velocity.x,
                             imu_Message_data->angular_velocity.y,
                             imu_Message_data->angular_velocity.z;

    //ROS_INFO("Acceleration_x: %f",imu_Message_data->linear_acceleration.x);
    eskf_.predict(raw_acceleration_measurements,raw_gyro_measurements,Ts);       
              
}


void ESKF_Node::publishPoseState(const ros::TimerEvent&)
{
    nav_msgs::Odometry odom_msg;
    static size_t trace_id{0};
    const VectorXd& pose = eskf_.getPose();
    const MatrixXd& errorCovariance = eskf_.getErrorCovariance();

    odom_msg.header.frame_id = "/eskf_link";
    odom_msg.header.seq = trace_id++;
    odom_msg.header.stamp = ros::Time::now();
    odom_msg.pose.pose.position.x = pose(StateMemberX);
    odom_msg.pose.pose.position.y = pose(StateMemberY);
    odom_msg.pose.pose.position.z = pose(StateMemberZ);
    odom_msg.twist.twist.linear.x = pose(StateMemberVx);
    odom_msg.twist.twist.linear.y = pose(StateMemberVy);
    odom_msg.twist.twist.linear.z = pose(StateMemberVz);
    odom_msg.pose.pose.orientation.w = pose(StateMemberQw);
    odom_msg.pose.pose.orientation.x = pose(StateMemberQx);
    odom_msg.pose.pose.orientation.y = pose(StateMemberQy);
    odom_msg.pose.pose.orientation.z = pose(StateMemberQz);
    //odom_msg.pose.covariance

    //loadParametersFromYamlFile();


    /*
    // Position covariance
    for(size_t i = 0; i < NOMINAL_POSITION_STATE_SIZE;i++)
    {
        for(size_t j = 0; j<NOMINAL_POSITION_STATE_SIZE;j++)
        {
            odom_msg.pose.covariance[NOMINAL_POSITION_STATE_SIZE*i+j] = errorCovariance(i,j);
        }
    }

    for(size_t i = 0; i < NOMINAL_VELOCITY_STATE_SIZE; i++)
    {
        for(size_t j = 0; j< NOMINAL_VELOCITY_STATE_SIZE; j++)
        {
            odom_msg.pose.covariance[(NOMINAL_VELOCITY_STATE_SIZE*i+j)+8] = errorCovariance(i + NOMINAL_VELOCITY_STATE_OFFSET, j + NOMINAL_VELOCITY_STATE_OFFSET);
        }
    }
    */
    
    /*
     for (size_t i = 0; i < POSE_SIZE; i++)
      {
        for (size_t j = 0; j < POSE_SIZE; j++)
        {
          message.pose.covariance[POSE_SIZE * i + j] = estimateErrorCovariance(i, j);
        }
      }
    */

    //ROS_INFO("StateX: %f",odom_msg.pose.pose.position.x);
    publishPose_.publish(odom_msg);

    
    //std::cout<<pose<<std::endl;
    //std::cout<<std::endl;

}



#include <iostream>
#include<math.h>
#include<vector>
#include<algorithm>
#include<limits.h>
#include<float.h>

using namespace std;
//parameters
int area_x = 100;
int area_y = 100;
int curr_round = 1;
int max_round = 4000;
int dead_nodes = 0;
int total_nodes = 150;
int network_status = 1;
int alive_nodes = total_nodes;
long double p = 0.1;
//energy model
float Eo = 1.0f; // initital energy0f//Eelec=Etx=Erx
long double ETX = 50 * 0.000000001;
long double ERX = 50 * 0.000000001;
//Transmit Amplifier types
long double Efs = 0.000000000010;
long double Emp = 0.0013 * 0.000000000001;
//Data Aggregation Energy
long double EDA = 5 * 0.000000001;
//do calculation
long double d = sqrt(Efs / Emp);

//number of rounds
int rmax = 10;

int data_packet_len = 4000;
int hello_packet_len = 100;
int num_packet = 10;
//radio range
long double RR = 0.5 * area_x * sqrt(2);
vector<int> gdata;
//node class
class Node
{
    float power = Eo, x, y, status = 1;
public:
    int id, ch_id, type = 0;
    Node(int X, int Y, int ID)
    {
        x = X;
        y = Y;
        id = ID;
    }
    float power_status()
    {
        return power;
    }
    void update_power(long double power, float loss = 0.0f)
    {
        //cout << type << "\\" << id << "->" << this->power << "-->"<<power<<"|=>"<<loss << endl;
        //this->power = power;
        //cout <<type<<"\\" << id << "->" << power << endl;
        long double temp = (double)this->power;
        this->power = (float)(temp - (double)loss);

    }
    void getData()
    {
        //cout<<id<<endl;
        cout << x << " " << y;
    }
    int x_location()
    {
        return x;
    }
    int y_location()
    {
        return y;
    }
    int get_status()
    {
        if (power <= 0)
        {
            status = 0;
        }
        return status;
    }
    void update_cl(int ch_id)
    {
        this->ch_id = ch_id;
    }



};
//sink class
class Sink
{

public:

    int x = 0;
    int y = 0;


};



//sensor generation

void generate_sensor(vector<Node*>& node_list, int x, int y)
{
    for (int i = 0; i < total_nodes; i++)
    {
        int range = x;
        int x_temp = (rand() % range) + 1;
        int y_temp = (rand() % range) + 1;


        /*int x_temp = (rand() % x) + 1;
        int y_temp = (rand() % y) + 1;*/
        // // Node* temp= new Node(x_temp,y_temp,i);

        // // node_list.push_back(Node(x_temp,y_temp,i));
        node_list[i] = new Node(x_temp, y_temp, i);
        // cout<<node_list[i]->getData();




    }
}
//calculate average distance to the base station from sensor nodes
float avg_dist_among_sensor_basestation(vector<Node*>& node_list)
{

    float dist = 0;
    int count = 0;
    for (int i = 0; i < node_list.size(); i++)
    {
        if (node_list[i]->get_status() == 1)
        {
            count++;
            int x1 = node_list[i]->x_location();
            int y1 = node_list[i]->y_location();
            float temp = (float)sqrt(pow(abs(0 - x1), 2) + pow(abs(0 - y1), 2) * 1.0);
            dist = dist + temp;
        }
    }

    return (dist / count);
}
//calculate Ech
long double Ech_calc(long double kopt)
{
    long double temp = (ERX + data_packet_len * EDA) * (alive_nodes / kopt);
    // cout<<temp;
    return temp;
}
//T(n) value

long double tn_calc()
{

    int temp = 10; // (1/p)=10

    long double temp2 = curr_round % temp;

    long double r = (long double)1.00 - (long double)p;

    long double t = p / (1 - (p * temp2));
    //cout << t << endl;
    return t;


}

//cluster head generation
void cluster_head_generation(vector<Node*> node_list, vector<Node*>& cluster_head_list)
{
    alive_nodes = total_nodes - dead_nodes;
    if (alive_nodes <= 0)
    {
        network_status = 0;
        return;
    }
    long double pi = 3.14;
    float d_ = avg_dist_among_sensor_basestation(node_list);
    long double kopt=(sqrt(alive_nodes/(2*pi)))*(sqrt(Efs/Emp)*(area_x/(pow(d_,2))));

    //long double kopt = 0.1 * alive_nodes;
    kopt = ceil(kopt);
    //cout <<"cluster head: "<< kopt << endl;
    //p = kopt / alive_nodes;
    int G = 0;
    // cout<<kopt<<endl;
    //when first round


        //Ech calculation 
    int temp_ech = Ech_calc(kopt);

    long double temp_tn = tn_calc();

    for (int i = 0; i < node_list.size(); i++)
    {
        if (G == kopt)
        {
            break;
        }
        float Eres = node_list[i]->power_status();
        if (Eres >= temp_ech && node_list[i]->type == 0 && node_list[i]->get_status() == 1)
        {
            long double rndm = (long double)rand() / (long double)RAND_MAX;
            //cout << rndm <<" "<< temp_tn <<endl;
            if (rndm < temp_tn)
            {
                cluster_head_list.push_back(node_list[i]);
                node_list[i]->type = 1;
                G++;
            }

        }





    }
    //cout << G << " -- " << kopt << endl;





}
//count Dead nodes
void dead_nodes_count(vector<Node*> node_list)
{
    dead_nodes = 0;
    for (int i = 0; i < node_list.size(); i++)
    {

        if (node_list[i]->get_status() == 0)
        {
            dead_nodes++;
        }




    }
    alive_nodes = total_nodes - dead_nodes;
    if (alive_nodes == 0)
    {
        network_status = 0;
    }
    //cout << "current round: " << curr_round << " Alive nodes: " << alive_nodes << " Dead nodes: " << dead_nodes << endl;
    gdata.push_back(alive_nodes);
}
//data tranfer
void send_recive_data(vector<Node*>& node_list, int packet_size, int sender_id, int receiver_id, int type)
{
    //type 0= sensor to sensor type 1= sensor to sink

    int x1 = node_list[sender_id]->x_location();
    int y1 = node_list[sender_id]->y_location();
    int x2, y2, flag = 0;
    if (type == 0)
    {
        x2 = node_list[receiver_id]->x_location();
        y2 = node_list[receiver_id]->y_location();
    }
    else
    {
        x2 = 0;
        y2 = 0;
    }

    //cout << x1 << "," << y1 << "-" << x2 << "," << y2 << " ";

    long double distance = sqrt(pow(abs(x2 - x1), 2) + pow(abs(y2 - y1), 2) * 1.0);
    long double dist_to_sink = sqrt(pow(abs(0 - x1), 2) + pow(abs(0 - y1), 2) * 1.0);


    if (dist_to_sink < distance)
    {
        flag = 1;
        distance = dist_to_sink;
    }
    //cout << "dist-" << distance << endl;
    float temp_power = node_list[sender_id]->power_status();
    float loss;
    //cout << d << endl;
    //message sending power loss
    if (distance > d)
    {


        loss = (float)(ETX * packet_size + Emp * packet_size * (pow(d, 4)));
    }
    else {
        loss = (float)(ETX * packet_size + Efs * packet_size * (pow(d, 2)));
    }
    //cout << "loss-" << loss << endl;
    node_list[sender_id]->update_power((float)((double)temp_power - (double)loss), loss);
    node_list[sender_id]->get_status();


    //message receiving power loss

    if (type == 0 && !flag)
    {
        temp_power = node_list[receiver_id]->power_status();
        //loss = (ERX + EDA) * packet_size;
        loss = (ERX)*packet_size;
        node_list[receiver_id]->update_power(temp_power - loss, loss);
        node_list[receiver_id]->get_status();
        flag = 0;

    }




}

//void send_recive_data(vector<Node*>& node_list, int packet_size, int sender_id, int receiver_id, int type)
//{
//    //type 0= sensor to sensor type 1= sensor to sink
//
//    int x1 = node_list[sender_id]->x_location();
//    int y1 = node_list[sender_id]->y_location();
//    int x2, y2;
//    if (type == 0)
//    {
//        x2 = node_list[receiver_id]->x_location();
//        y2 = node_list[receiver_id]->y_location();
//    }
//    else
//    {
//        x2 = 0;
//        y2 = 0;
//    }
//
//    //cout << x1 << "," << y1 << "-" << x2 << "," << y2 << " ";
//
//    long double distance = sqrt(pow(abs(x2 - x1), 2) + pow(abs(y2 - y1), 2) * 1.0);
//
//    long double dis_to_sink = sqrt(pow(abs(0 - x1), 2) + pow(abs(0 - y1), 2) * 1.0);
//
//    distance = min(distance, dis_to_sink);
//    //cout << "dist-" << distance << endl;
//    long double temp_power = node_list[sender_id]->power_status();
//    long double loss;
//    //cout << d << endl;
//    //message sending power loss
//    if (distance > d)
//    {
//
//
//        loss = ETX * packet_size + Emp * packet_size * (pow(d, 4));
//    }
//    else {
//        loss = ETX * packet_size + Efs * packet_size * (pow(d, 2));
//    }
//    //cout << "loss-" << loss << endl;
//    node_list[sender_id]->update_power(temp_power - loss,loss);
//    node_list[sender_id]->get_status();
//
//
//    //message receiving power loss
//
//    /*if (type == 0)
//    {
//        temp_power = node_list[receiver_id]->power_status();
//        loss = (ERX + EDA) * packet_size;
//        node_list[receiver_id]->update_power(temp_power - loss);
//        node_list[receiver_id]->get_status();
//
//    }*/
//
//
//
//
//}
void receive_data(vector<Node*>& cluster_head_list, int packet_size, int receiver_id, int type)
{

    int temp_power = cluster_head_list[receiver_id]->power_status();
    int loss = (ERX + EDA) * packet_size;
    cluster_head_list[receiver_id]->update_power(temp_power - loss, loss);
    cluster_head_list[receiver_id]->get_status();

}

void clustering(vector<Node*>& node_list, vector<Node*> cluster_head_list)
{
    for (int i = 0; i < node_list.size(); i++)
    {

        if (node_list[i]->get_status() == 1 && node_list[i]->type == 0)
        {
            int curr_dist = INT_MAX;
            int my_x = node_list[i]->x_location();
            int my_y = node_list[i]->y_location();

            for (int j = 0; j < cluster_head_list.size(); j++)
            {
                int cl_x = cluster_head_list[j]->x_location();
                int cl_y = cluster_head_list[j]->y_location();
                long double distance = sqrt(pow(abs(cl_x - my_x), 2) + pow(abs(cl_y - my_y), 2) * 1.0);
                if (curr_dist > distance)
                {
                    curr_dist = distance;
                    node_list[i]->update_cl(cluster_head_list[j]->id);
                }
            }
        }


    }
}
void clusterHead_to_sink(vector<Node*>& node_list, vector<Node*>& cluster_head_list, int& packets)
{
    for (int i = 0; i < cluster_head_list.size(); i++)
    {
        packets++;
        send_recive_data(node_list, data_packet_len, cluster_head_list[i]->id, 0, 1);
    }
}
void clusterMember_to_clusterHead(vector<Node*>& node_list, int& packets)
{
    for (int i = 0; i < node_list.size(); i++)
    {
        if (node_list[i]->type == 0 && node_list[i]->get_status() == 1)
        {
            packets++;
            send_recive_data(node_list, data_packet_len, node_list[i]->id, node_list[i]->ch_id, 0);
        }

    }

}

//void clusterMember_to_clusterHead(vector<Node*>& node_list, vector<Node*>& cluster_head_list, int& packets)
//{
//    for (int i = 0; i < node_list.size(); i++)
//    {
//        if (node_list[i]->type == 0 && node_list[i]->get_status() == 1)
//        {
//            packets++;
//            send_recive_data(node_list, data_packet_len, node_list[i]->id, node_list[i]->ch_id, 0);
//        }
//
//    }
//
//    for (int i = 0; i < cluster_head_list.size(); i++)
//    {
//        if (cluster_head_list[i]->get_status() == 1)
//        {
//            
//            receive_data(cluster_head_list, data_packet_len, node_list[i]->id, 0);
//        }
//
//    }
//
//}


//reset sensors

void reset_sensors(vector<Node*>& node_list)
{
    for (int i = 0; i < node_list.size(); i++)
    {
        node_list[i]->type = 0;
        node_list[i]->ch_id = 0;
    }
}

void simulate(vector<Node*>& node_list, vector<Node*>& cluster_head_list)
{

    while (curr_round <= max_round && alive_nodes > 0)
    {


        reset_sensors(node_list);
        //cout <<"===================================="<<endl;
        cluster_head_list.clear();
        int packets = 0;
        dead_nodes_count(node_list);
        cluster_head_generation(node_list, cluster_head_list);
        clustering(node_list, cluster_head_list);
        clusterMember_to_clusterHead(node_list, packets);
        //clusterMember_to_clusterHead(node_list,cluster_head_list, packets);
        clusterHead_to_sink(node_list, cluster_head_list, packets);

        /*for (int i = 0; i < node_list.size(); i++)
        {
            if (node_list[i]->type == 0)
            {
                cout << " , " << node_list[i]->id;
            }
        }
        cout << endl;
        cout << "cluster heads" << endl;
        for (int i = 0; i < cluster_head_list.size(); i++)
        {

                cout << " , " << cluster_head_list[i]->id;

        }
        cout << endl;*/

        //cout << packets << endl;
        curr_round++;
    }

}

int main()
{
    vector<Node*> node_list(total_nodes);
    vector<Node*> cluster_head_list;
    Sink snk;
    //parameters


    generate_sensor(node_list, area_x, area_y);
    simulate(node_list, cluster_head_list);

    //cout << "Round -- Alive nodes" << endl;

    for (int i = 0; i < gdata.size(); i++)
    {
        cout << i + 1 << " " << gdata[i] << endl;
        //cout << gdata[i] << endl;
    }


    /* cout<<dead_nodes<<endl;*/


    // send_recive_data(node_list,data_packet_len,1,8);
    // cout<<node_list[1]->power_status();
    // cout<<endl;
    // cout<<node_list[8]->power_status();
    // cout<<endl;




    // for(int i=0;i<cluster_head_list.size();i++)
    // {

    //     cout<<cluster_head_list[i]->id;
    //     cout<<endl;
    // }
    // for(int i=0;i<node_list.size();i++)
    // {

    //     cout<<node_list[i]->id<<"->"<<node_list[i]->ch_id;
    //     cout<<endl;
    // }


    /*for (int i = 0; i < node_list.size(); i++)
    {

        cout << node_list[i]->power_status();
        cout << endl;
    }*/



}
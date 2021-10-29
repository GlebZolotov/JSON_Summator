/*
 * logging.cpp
 *
 *  Created on: 5 сент. 2021 г.
 *      Author: gleb
 */

#include "logging.hpp"

IP7_Trace * init_logger(IP7_Client **l_pClient) {
    IP7_Trace         *l_pTrace     = NULL;

    P7_Set_Crash_Handler();
    //create P7 client object
    *l_pClient = P7_Create_Client(TM(ParamsOfClient.c_str()));
    if (NULL == *l_pClient) return NULL;

    //create P7 trace object 1
    l_pTrace = P7_Create_Trace(*l_pClient, TM(NameOfTrace.c_str()));
    if (NULL == l_pTrace) {
        (*l_pClient)->Release();
        l_pClient = NULL;
        return NULL;
    }
    l_pTrace->Share(TM(NameOfTrace.c_str()));

    return l_pTrace;
}

IP7_Trace * connect_logger() {
    return P7_Get_Shared_Trace(TM(NameOfTrace.c_str()));
}

void reg_current_thread(IP7_Trace * tr, const char *name_of_thr) {
    tr->Register_Thread(TM(name_of_thr), 0);    
}

void del_current_thread(IP7_Trace * tr) {
    tr->Unregister_Thread(0);
    tr->Release();
    tr = NULL;
}

void close_logger(IP7_Client *l_pClient) {
    l_pClient->Release();
    l_pClient = NULL;
}




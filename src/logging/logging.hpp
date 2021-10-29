#pragma once

#include <stdlib.h>
#include <string>
#include "P7_Client.h"
#include "P7_Trace.h"

const std::string NameOfTrace("Application");
const std::string ParamsOfClient("/P7.Sink=Console /P7.Format=\"%tm %lv Thr:%tn(%ti) Core:%cc Place %fs %fl %fn: %ms\"");

IP7_Trace * init_logger(IP7_Client **l_pClient);
IP7_Trace * connect_logger();
void reg_current_thread(IP7_Trace * tr, const char *name_of_thr);
void del_current_thread(IP7_Trace * tr);
void close_logger(IP7_Client *l_pClient);




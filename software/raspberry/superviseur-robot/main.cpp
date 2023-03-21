/*
 * Copyright (C) 2018 dimercur
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <unistd.h>

#include <sys/mman.h>

#ifdef __WITH_PTHREAD__
#include "tasks_pthread.h"
#else
#include "tasks.h"
#endif // __WITH_PTHREAD__


// Włączenie serwera ComMonitor serv = launchServer();
/*
ComMonitor launchServer(void) {
    ComMonitor serv =  new ComMonitor() ;
    if (serv.Open != 1) {
        cout << "Server error" << endl ;
        exit(EXIT_FAILURE);
    }
    return serv ;
}
  */

int main(int argc, char **argv) {
    Tasks tasks;
    //Lock the memory to avoid memory swapping for this program
    mlockall(MCL_CURRENT | MCL_FUTURE);

    cout<<"#################################"<<endl;
    cout<<"#      DE STIJL PROJECT         #"<<endl;
    cout<<"#################################"<<endl;

//    ComMonitor serv = launchServer();

    tasks.Init();
    tasks.Run();
    tasks.Join();
    
    //tasks.Run();

    // TODO: remove this or the accept in the while loop
//    cout << "Waiting for client to accept..." << endl;
//    serv.AcceptClient() ;
//    cout << "Client accepted! Ready to receive messages!" << endl;
//
//    while (true) {
//        Message *msg = serv.Read();
//        cout << "Received message : " ;
//        
//        switch (*msg) {
//        case MESSAGE_ROBOT_COM_OPEN:
//            // TODO: what do we do if we receive a com open message again?
//            cout << "Opening communication (id=" << MESSAGE_ROBOT_COM_OPEN << ")" << endl;
//            serv.AcceptClient() ;
//            serv.Write(MESSAGE_ANSWER_ACK) ;
//            break;
//        case MESSAGE_ROBOT_COM_CLOSE:
//            serv.Close();
//            serv.Write(MESSAGE_ANSWER_ACK) ;
//            break;
//        case MESSAGE_ROBOT_START_WITH_WD: case MESSAGE_ROBOT_START_WITHOUT_WD:
//            serv.Write(MESSAGE_ANSWER_ACK) ;
//            break;
//        case MESSAGE_CAM_OPEN:
//            serv.Write(MESSAGE_ANSWER_ACK) ;
//            break;
//        case MESSAGE_CAM_CLOSE:
//            serv.Write(MESSAGE_ANSWER_ACK) ;
//            break;
//        case MESSAGE_CAM_ASK_ARENA:
//            break;
//        case MESSAGE_CAM_ASK_ARENA:
//            break;
//        case MESSAGE_CAM_ARENA_CONFIRM:
//            break;
//        case MESSAGE_CAM_ARENA_INFIRM:
//            break;
//        case MESSAGE_CAM_POSITION_COMPUTE_START:
//            break;
//        case MESSAGE_CAM_POSITION_COMPUTE_STOP:
//            break;
//
//        // If timedout, ComMonitor::Read retruns a MESSAGE_MONITOR_LOST
//        case MESSAGE_MONITOR_LOST:
//        break;
//
//        default:
//            serv.Write(MESSAGE_ANSWER_NACK) ;
//            break;
//        }
    
//    }

    tasks.Stop();
    return 0;
}


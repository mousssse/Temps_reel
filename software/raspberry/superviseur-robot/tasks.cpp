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

#include "tasks.h"
#include <stdexcept>

// Déclaration des priorités des taches
#define PRIORITY_TSERVER 30
#define PRIORITY_TOPENCOMROBOT 20
#define PRIORITY_TMOVE 26
#define PRIORITY_TSENDTOMON 25
#define PRIORITY_TRECEIVEFROMMON 25
#define PRIORITY_TSTARTROBOT 20
#define PRIORITY_TCAMERA 21
#define PRIORITY_TBATTERY 18
#define PRIORITY_TSTARTCAMERA 20
#define PRIORITY_TGRABCAMERA 25
#define PRIORITY_TSTOPCAMERA 20
#define PRIORITY_TSEARCHARENA 23
#define PRIORITY_TSEARCHROBOT 18
#define PRIORITY_RELOAD_WD 23

/*
 * Some remarks:
 * 1- This program is mostly a template. It shows you how to create tasks, semaphore
 *   message queues, mutex ... and how to use them
 * 
 * 2- semDumber is, as name say, useless. Its goal is only to show you how to use semaphore
 * 
 * 3- Data flow is probably not optimal
 * 
 * 4- Take into account that ComRobot::Write will block your task when serial buffer is full,
 *   time for internal buffer to flush
 * 
 * 5- Same behavior existe for ComMonitor::Write !
 * 
 * 6- When you want to write something in terminal, use cout and terminate with endl and flush
 * 
 * 7- Good luck !
 */

/**
 * @brief Initialisation des structures de l'application (tâches, mutex, 
 * semaphore, etc.)
 */
void Tasks::Init() {
    int status;
    int err;

    /**************************************************************************************/
    /* 	Mutex creation                                                                    */
    /**************************************************************************************/
    if (err = rt_mutex_create(&mutex_monitor, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_robot, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_robotStarted, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_move, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_camera, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_cameraStarted, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_img, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_grab, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_arena, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_robotPosition, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_mutex_create(&mutex_WD, NULL)) {
        cerr << "Error mutex create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Mutexes created successfully" << endl << flush;

    /**************************************************************************************/
    /* 	Semaphors creation       							  */
    /**************************************************************************************/
    if (err = rt_sem_create(&sem_barrier, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_openComRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_serverOk, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_startRobot, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_startCamera, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_stopCamera, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_sem_create(&sem_searchArena, NULL, 0, S_FIFO)) {
        cerr << "Error semaphore create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Semaphores created successfully" << endl << flush;

    /**************************************************************************************/
    /* Tasks creation                                                                     */
    /**************************************************************************************/
    if (err = rt_task_create(&th_server, "th_server", 0, PRIORITY_TSERVER, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_sendToMon, "th_sendToMon", 0, PRIORITY_TSENDTOMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_receiveFromMon, "th_receiveFromMon", 0, PRIORITY_TRECEIVEFROMMON, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_openComRobot, "th_openComRobot", 0, PRIORITY_TOPENCOMROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_startRobot, "th_startRobot", 0, PRIORITY_TSTARTROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_reloadWD, "th_reloadWD", 0, PRIORITY_RELOAD_WD, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_move, "th_move", 0, PRIORITY_TMOVE, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_battery, "th_battery", 0, PRIORITY_TBATTERY, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_startCamera, "th_startCamera", 0, PRIORITY_TSTARTCAMERA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_grabCamera, "th_grabCamera", 0, PRIORITY_TGRABCAMERA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_stopCamera, "th_stopCamera", 0, PRIORITY_TSTOPCAMERA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_searchArena, "th_searchArena", 0, PRIORITY_TSEARCHARENA, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_create(&th_searchRobot, "th_searchRobot", 0, PRIORITY_TSEARCHROBOT, 0)) {
        cerr << "Error task create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Tasks created successfully" << endl << flush;

    /**************************************************************************************/
    /* Message queues creation                                                            */
    /**************************************************************************************/
    if ((err = rt_queue_create(&q_messageToMon, "q_messageToMon", sizeof (Message*)*50, Q_UNLIMITED, Q_FIFO)) < 0) {
        cerr << "Error msg queue create: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    cout << "Queues created successfully" << endl << flush;

}

/**
 * @brief Démarrage des tâches
 */
void Tasks::Run() {
    rt_task_set_priority(NULL, T_LOPRIO);
    int err;

    if (err = rt_task_start(&th_server, (void(*)(void*)) & Tasks::ServerTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_sendToMon, (void(*)(void*)) & Tasks::SendToMonTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_receiveFromMon, (void(*)(void*)) & Tasks::ReceiveFromMonTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_openComRobot, (void(*)(void*)) & Tasks::OpenComRobot, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_startRobot, (void(*)(void*)) & Tasks::StartRobotTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_move, (void(*)(void*)) & Tasks::MoveTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_battery, (void(*)(void*)) & Tasks::ShowBatteryTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_startCamera, (void(*)(void*)) & Tasks::StartCameraTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_grabCamera, (void(*)(void*)) & Tasks::GrabTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_stopCamera, (void(*)(void*)) & Tasks::StopCameraTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_searchArena, (void(*)(void*)) & Tasks::SearchArenaTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_searchRobot, (void(*)(void*)) & Tasks::SearchRobotTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }
    if (err = rt_task_start(&th_reloadWD, (void(*)(void*)) & Tasks::ReloadWDTask, this)) {
        cerr << "Error task start: " << strerror(-err) << endl << flush;
        exit(EXIT_FAILURE);
    }

    cout << "Tasks launched" << endl << flush;
}

/**
 * @brief Arrêt des tâches
 */
void Tasks::Stop() {
    monitor.Close();
    robot.Close();
    if (camera.IsOpen()) {
        camera.Close() ;
    }
}

/**
 */
void Tasks::Join() {
    cout << "Tasks synchronized" << endl << flush;
    rt_sem_broadcast(&sem_barrier);
    pause();
}

/**
 * @brief Thread handling server communication with the monitor.
 */
void Tasks::ServerTask(void *arg) {
    int status;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are started)
    rt_sem_p(&sem_barrier, TM_INFINITE);

    /**************************************************************************************/
    /* The task server starts here                                                        */
    /**************************************************************************************/
    rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
    status = monitor.Open(SERVER_PORT);
    rt_mutex_release(&mutex_monitor);

    cout << "Open server on port " << (SERVER_PORT) << " (" << status << ")" << endl;

    if (status < 0) throw std::runtime_error {
        "Unable to start server on port " + std::to_string(SERVER_PORT)
    };
    monitor.AcceptClient(); // Wait the monitor client
    cout << "Rock'n'Roll baby, client accepted!" << endl << flush;
    rt_sem_broadcast(&sem_serverOk);
}

/**
 * @brief Thread sending data to monitor.
 */
void Tasks::SendToMonTask(void* arg) {
    Message *msg;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);

    /**************************************************************************************/
    /* The task sendToMon starts here                                                     */
    /**************************************************************************************/
    rt_sem_p(&sem_serverOk, TM_INFINITE);

    while (1) {
        cout << "wait msg to send" << endl << flush;
        msg = ReadInQueue(&q_messageToMon);
        cout << "Send msg to mon: " << msg->ToString() << endl << flush;
        rt_mutex_acquire(&mutex_monitor, TM_INFINITE);
        monitor.Write(msg); // The message is deleted with the Write
        rt_mutex_release(&mutex_monitor);
    }
}

/**
 * @brief Thread receiving data from monitor.
 */
void Tasks::ReceiveFromMonTask(void *arg) {
    Message *msgRcv;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task receiveFromMon starts here                                                */
    /**************************************************************************************/
    rt_sem_p(&sem_serverOk, TM_INFINITE);
    cout << "Received message from monitor activated" << endl << flush;
    
    while (1) {
        msgRcv = monitor.Read();
        cout << "Rcv <= " << msgRcv->ToString() << endl << flush;

        if (msgRcv->CompareID(MESSAGE_MONITOR_LOST)) {
            robot.Close();
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            robotStarted = 0;
            rt_mutex_release(&mutex_robotStarted);
            monitor.AcceptClient(); // Wait the monitor client
            cout << "Rock'n'Roll baby, client accepted!" << endl << flush;
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_COM_OPEN)) {
            rt_sem_v(&sem_openComRobot);
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITHOUT_WD)) {
            rt_mutex_acquire(&mutex_WD, TM_INFINITE);
            usingWD = 0;
            rt_mutex_release(&mutex_WD);
            rt_sem_v(&sem_startRobot);
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITH_WD)) {
            rt_mutex_acquire(&mutex_WD, TM_INFINITE);
            usingWD = 1;
            rt_mutex_release(&mutex_WD);
            rt_sem_v(&sem_startRobot);
        } else if (msgRcv->CompareID(MESSAGE_ROBOT_GO_FORWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_BACKWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_LEFT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_RIGHT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_STOP)) {

            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            move = msgRcv->GetID();
            rt_mutex_release(&mutex_move);
        } else if (msgRcv -> CompareID(MESSAGE_CAM_OPEN)) {
            rt_sem_v(&sem_startCamera) ;
        } else if (msgRcv -> CompareID(MESSAGE_CAM_CLOSE)) {
            rt_sem_v(&sem_stopCamera) ;
        } else if (msgRcv -> CompareID(MESSAGE_CAM_ASK_ARENA)) {
            rt_sem_v(&sem_searchArena) ;
        } else if (msgRcv -> CompareID(MESSAGE_CAM_ARENA_INFIRM)) {
            rt_mutex_acquire(&mutex_arena, TM_INFINITE);
            arena = Arena() ;
            rt_mutex_release(&mutex_arena);
            rt_mutex_acquire(&mutex_grab, TM_INFINITE);
            grabImage = 1;
            rt_mutex_release(&mutex_grab);
        } else if (msgRcv -> CompareID(MESSAGE_CAM_ARENA_CONFIRM)) {
            rt_mutex_acquire(&mutex_grab, TM_INFINITE);
            grabImage = 1;
            rt_mutex_release(&mutex_grab);
        } else if (msgRcv -> CompareID(MESSAGE_CAM_POSITION_COMPUTE_START)) {
            rt_mutex_acquire(&mutex_robotPosition, TM_INFINITE);
            robotPosition = 1;
            rt_mutex_release(&mutex_robotPosition);
        }
        else if (msgRcv -> CompareID(MESSAGE_CAM_POSITION_COMPUTE_STOP)) {
            rt_mutex_acquire(&mutex_robotPosition, TM_INFINITE);
            robotPosition = 0;
            rt_mutex_release(&mutex_robotPosition);
        }
        
        delete(msgRcv); // mus be deleted manually, no consumer
    }
}

/**
 * @brief Thread opening communication with the robot.
 */
void Tasks::OpenComRobot(void *arg) {
    int status;
    int err;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task openComRobot starts here                                                  */
    /**************************************************************************************/
    while (1) {
        rt_sem_p(&sem_openComRobot, TM_INFINITE);
        cout << "Open serial com (";
        rt_mutex_acquire(&mutex_robot, TM_INFINITE);
        status = robot.Open();
        rt_mutex_release(&mutex_robot);
        cout << status;
        cout << ")" << endl << flush;

        Message * msgSend;
        if (status < 0) {
            msgSend = new Message(MESSAGE_ANSWER_NACK);
        } else {
            msgSend = new Message(MESSAGE_ANSWER_ACK);
        }
        WriteInQueue(&q_messageToMon, msgSend); // msgSend will be deleted by sendToMon
    }
}

/**
 * @brief Thread starting the communication with the robot.
 */
void Tasks::StartRobotTask(void *arg) {
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    int watchdog;
    
    /**************************************************************************************/
    /* The task startRobot starts here                                                    */
    /**************************************************************************************/
    while (1) {

        Message * msgSend;
        rt_sem_p(&sem_startRobot, TM_INFINITE);
        
        rt_mutex_acquire(&mutex_WD, TM_INFINITE);
        watchdog = usingWD;
        rt_mutex_release(&mutex_WD);
        // while count < 3
        if (!watchdog) {
            cout << "Start robot without watchdog (";
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            msgSend = robot.Write(robot.StartWithoutWD());
            rt_mutex_release(&mutex_robot);
        }
        else {
            cout << "Start robot with watchdog (";
            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            msgSend = robot.Write(robot.StartWithWD());
            updateRobotErrorCount(msgSend);
            rt_mutex_release(&mutex_robot);
        }
        
        cout << msgSend->GetID();
        cout << ")" << endl;

        cout << "Movement answer: " << msgSend->ToString() << endl << flush;
        WriteInQueue(&q_messageToMon, msgSend);  // msgSend will be deleted by sendToMon
        
        if (msgSend->GetID() == MESSAGE_ANSWER_ACK) {
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            robotStarted = 1;
            rt_mutex_release(&mutex_robotStarted);
        }
    }
}

void Tasks::updateRobotErrorCount(Message * msgSend) {
    if (msgSend->GetID() == MESSAGE_ANSWER_ACK) {
        robotErrorCount = 0;
    }
    else {
        if (robotErrorCount++ >=2) {
            cout << "Connection to robot lost" << endl << flush;;
            robot.Close();
            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
            robotStarted = 0;
            rt_mutex_release(&mutex_robotStarted);
        }
    }
}

/**
 * @brief Thread handling control of the robot.
 */
void Tasks::MoveTask(void *arg) {
    int rs;
    int cpMove;
    
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;
    // Synchronization barrier (waiting that all tasks are starting)
    rt_sem_p(&sem_barrier, TM_INFINITE);
    
    /**************************************************************************************/
    /* The task starts here                                                               */
    /**************************************************************************************/
    rt_task_set_periodic(NULL, TM_NOW, 100000000);

    while (1) {
        rt_task_wait_period(NULL);
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        if (rs == 1) {
            cout << "Periodic movement update";
            rt_mutex_acquire(&mutex_move, TM_INFINITE);
            if (cpMove!=MESSAGE_ROBOT_STOP || move!=MESSAGE_ROBOT_STOP) {
                cpMove = move;

                //cout << " move: " << cpMove;

                rt_mutex_acquire(&mutex_robot, TM_INFINITE);
                robot.Write(new Message((MessageID)cpMove));
                rt_mutex_release(&mutex_robot);
            }
            rt_mutex_release(&mutex_move);
        }
        //cout << endl << flush;
    }
}

void Tasks::ShowBatteryTask (void *arg) {
    // Fonctionnality 13
    // TODO NE PAS OUBLIER DE REMETTRE
    rt_task_set_periodic(NULL, TM_NOW, 500000000) ;
    int rs ;
    
    while(1) {
        rt_task_wait_period(NULL);
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        
        if (rs==1) {
            //cout << "Mise à jour de la batterie " << endl << flush;

            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            Message* message = robot.Write(robot.GetBattery()) ;
            updateRobotErrorCount(msgSend);
            rt_mutex_release(&mutex_robot);

            if (message->GetID() == MESSAGE_ROBOT_BATTERY_LEVEL) {
                WriteInQueue(&q_messageToMon, message);
            }
        }
    }
}

void Tasks::StartCameraTask (void *arg) {
    // Fonctionnality 14
    int rs;
    
    while(1) {
        rt_sem_p(&sem_startCamera, TM_INFINITE) ;
        
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        
        if (rs==1) {
            Message* msgSend ;
            rt_mutex_acquire(&mutex_camera, TM_INFINITE);
            if (camera.Open()) {
                msgSend = new Message(MESSAGE_ANSWER_ACK);
                rt_mutex_acquire(&mutex_cameraStarted, TM_INFINITE);
                cameraStarted = 1;
                rt_mutex_release(&mutex_cameraStarted);
                cout << "Camera opened!" << endl << flush;
            } else {
                msgSend = new Message(MESSAGE_ANSWER_NACK);
            }
            rt_mutex_release(&mutex_camera);
        
            WriteInQueue(&q_messageToMon, msgSend);
        }
        
    }    
}

void Tasks::GrabTask (void *arg) {
    // Fonctionnality 15
    int cs, gb, rp;
    rt_task_set_periodic(NULL, TM_NOW, 100000000);
    
    while(1) {
        rt_task_wait_period(NULL);
        
        rt_mutex_acquire(&mutex_cameraStarted, TM_INFINITE);
        cs = cameraStarted;
        rt_mutex_release(&mutex_cameraStarted);
        
        rt_mutex_acquire(&mutex_grab, TM_INFINITE);
        gb = grabImage;
        rt_mutex_release(&mutex_grab);
        
        rt_mutex_acquire(&mutex_robotPosition, TM_INFINITE);
        rp= robotPosition;
        rt_mutex_release(&mutex_robotPosition);
        
        if (cs == 1 && gb == 1) {
            rt_mutex_acquire(&mutex_camera, TM_INFINITE);
            rt_mutex_acquire(&mutex_img, TM_INFINITE) ;
            img = new Img(camera.Grab());
            if (!arena.IsEmpty()) {
                img->DrawArena(arena) ;
                if (rp == 1 && !position.empty()) {
                    img->DrawRobot(position.front()) ;
                }
            }
            
            MessageImg* msg = new MessageImg(MESSAGE_CAM_IMAGE, img) ;
            cout << "Image sent!" << endl << flush;
            
            WriteInQueue(&q_messageToMon, msg);
            
            rt_mutex_release(&mutex_img);
            rt_mutex_release(&mutex_camera);

        }

    }
}


void Tasks::StopCameraTask (void *arg) {
    // Fonctionnality 16
    int rs, cs;
    
    while(1) {
        rt_sem_p(&sem_stopCamera, TM_INFINITE) ;
        
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        
        rt_mutex_acquire(&mutex_cameraStarted, TM_INFINITE);
        cs = cameraStarted;
        rt_mutex_release(&mutex_cameraStarted);
        
        if (rs==1 && cs == 1) {
            Message* msgSend ;
            rt_mutex_acquire(&mutex_camera, TM_INFINITE);
            if (camera.IsOpen()) {
                camera.Close();
                msgSend = new Message(MESSAGE_ANSWER_ACK);
                rt_mutex_acquire(&mutex_cameraStarted, TM_INFINITE);
                cameraStarted = 0;
                rt_mutex_release(&mutex_cameraStarted);
                cout << "Camera closed!" << endl << flush;
            } else {
                msgSend = new Message(MESSAGE_ANSWER_NACK);
            }
            rt_mutex_release(&mutex_camera);
        
            WriteInQueue(&q_messageToMon, msgSend);
        }
        
    }
    
}


void Tasks::SearchArenaTask (void * arg) {
    
   int cs, rs ;
   while(1) {
        rt_sem_p(&sem_searchArena, TM_INFINITE) ;
        
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        
        rt_mutex_acquire(&mutex_cameraStarted, TM_INFINITE);
        cs = cameraStarted;
        rt_mutex_release(&mutex_cameraStarted);
        rt_mutex_acquire(&mutex_img, TM_INFINITE) ;
        
        if (cs==1 && rs ==1) {
            rt_mutex_acquire(&mutex_grab, TM_INFINITE);
            grabImage = 0;
            rt_mutex_release(&mutex_grab);
            
            rt_mutex_acquire(&mutex_arena, TM_INFINITE);
           // if (!arena.IsEmpty()) {
           //     cout << "BBBBBBBBBBBBBBBBBBBH" << endl << flush ;
            //}
            arena = img->SearchArena() ;
           // if (!arena.IsEmpty()) {
           //     cout << "CCCCCCCCCCCCCCCCCCh" << endl << flush ;
            //}
            Message* msgSend ;
            int i = 0 ;
            while(arena.IsEmpty() && i<10 ) {
                i++ ;
                msgSend = new Message(MESSAGE_ANSWER_NACK);
                WriteInQueue(&q_messageToMon, msgSend);
               // cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" <<endl << flush ;
                img = new Img(camera.Grab());
                MessageImg* msg = new MessageImg(MESSAGE_CAM_IMAGE, img) ;
                cout << "Image sent!" << endl << flush;
                WriteInQueue(&q_messageToMon, msg);
                arena = img->SearchArena() ;
            }
            img->DrawArena(arena) ;
            MessageImg* msg = new MessageImg(MESSAGE_CAM_IMAGE, img) ;
            WriteInQueue(&q_messageToMon, msg);
            rt_mutex_release(&mutex_arena);
            cout << "Arena draw" <<endl << flush ;

            
        }
        rt_mutex_release(&mutex_img);   
   }
}

void Tasks::SearchRobotTask( void * arg) {
    int cs, rs, rp ;
    rt_task_set_periodic(NULL, TM_NOW, 150000000) ;
   while(1) {
        rt_task_wait_period(NULL);
        rt_mutex_acquire(&mutex_robotPosition, TM_INFINITE);
        rp= robotPosition;
        rt_mutex_release(&mutex_robotPosition);
        
        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);
        
        rt_mutex_acquire(&mutex_cameraStarted, TM_INFINITE);
        cs = cameraStarted;
        rt_mutex_release(&mutex_cameraStarted);
        
        
        if (cs == 1 && rs == 1 && rp==1) {
            rt_mutex_acquire(&mutex_arena, TM_INFINITE);
            position = img->SearchRobot(arena) ;
            rt_mutex_release(&mutex_arena);                       
        } 
   }
}

/**
 * Write a message in a given queue
 * @param queue Queue identifier
 * @param msg Message to be stored
 */
void Tasks::WriteInQueue(RT_QUEUE *queue, Message *msg) {
    int err;
    if ((err = rt_queue_write(queue, (const void *) &msg, sizeof ((const void *) &msg), Q_NORMAL)) < 0) {
        cerr << "Write in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in write in queue"};
    }
}

/**
 * Read a message from a given queue, block if empty
 * @param queue Queue identifier
 * @return Message read
 */
Message *Tasks::ReadInQueue(RT_QUEUE *queue) {
    int err;
    Message *msg;

    if ((err = rt_queue_read(queue, &msg, sizeof ((void*) &msg), TM_INFINITE)) < 0) {
        cout << "Read in queue failed: " << strerror(-err) << endl << flush;
        throw std::runtime_error{"Error in read in queue"};
    }/** else {
        cout << "@msg :" << msg << endl << flush;
    } /**/

    return msg;
}

/**
 * Regularly sends a message to the robot to reload the watchdog
 */
void Tasks::ReloadWDTask(void *arg) {
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    // period of 1s
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);
    
    int rs;
    int wd;
    int count = 0;
    while (1) {
        rt_task_wait_period(NULL);

        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
        rs = robotStarted;
        rt_mutex_release(&mutex_robotStarted);

        rt_mutex_acquire(&mutex_WD, TM_INFINITE);
        wd = usingWD;
        rt_mutex_release(&mutex_WD);

        if (wd && rs) {
            cout << "Periodic watchdog update" << endl << flush;;
            count++;

            rt_mutex_acquire(&mutex_robot, TM_INFINITE);
            Message * msgSend = robot.Write(new Message(MESSAGE_ROBOT_RELOAD_WD));
            updateRobotErrorCount(msgSend);
            rt_mutex_release(&mutex_robot);
            WriteInQueue(&q_messageToMon, msgSend); 

            if (msgSend->GetID() == MESSAGE_ANSWER_ACK) {
                count--;
            }

            if (count >= 3) {
                cout << "Too many errors on watchdog - stopping robot" << endl << flush;;
                robot.Close();
                rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
                robotStarted = 0;
                rt_mutex_release(&mutex_robotStarted);
            }
        }
    }
}

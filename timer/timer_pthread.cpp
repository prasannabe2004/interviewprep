#include <iostream>
#include <pthread.h> /* To acquire mutex */
#include <queue>     /*To hold all the tasks which needs to be executed */
#include <unistd.h>  /* For Sleep */

using namespace std;

void Task1(void) {
    cout << "Task1 " << endl;
}
void Task2(void) {
    cout << "Task2 " << endl;
}
void Task3(void) {
    cout << "Task3 " << endl;
}
void Task4(void) {
    cout << "Task4 " << endl;
}
void Task5(void) {
    cout << "Task5 " << endl;
}

typedef struct {
    bool timer_enable;             /* Tells if timer is enabled or not */
    bool periodic;                 /* Tells if timer is periodic or not */
    uint32_t reload_timeout_value; /* Reload value of timer, used in case of periodic timer to
                                      reload curr timeout value*/
    uint32_t curr_timeout_value;   /* Curr value of timer */
    void (*Callback)(void);        /* Call Back Function to be called upon timeout */
} TimerTask_t;

#define MAX_TASK 5
TimerTask_t TimerTask[MAX_TASK];
queue<uint32_t> Task_queue;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void timer_stop(uint32_t timerId);

/* TimerInterruptFunction is called every 1 milli Sec */
void TimerInterruptFunction() {
    uint32_t timer_index;
    pthread_mutex_lock(&mtx);
    for (timer_index = 0; timer_index < MAX_TASK; timer_index++) {
        if (TimerTask[timer_index].timer_enable == true) {
            if (TimerTask[timer_index].curr_timeout_value == 0) {
                /* Stop the timer, if it is not periodic */
                if (TimerTask[timer_index].periodic == false) {
                    TimerTask[timer_index].timer_enable = false;
                } else {
                    /* Reload timer value */
                    TimerTask[timer_index].curr_timeout_value =
                        TimerTask[timer_index].reload_timeout_value;
                }
                /* Put this task into queue */
                Task_queue.push(timer_index);
            } else {
                TimerTask[timer_index].curr_timeout_value--;
            }
        }
    }
    pthread_mutex_unlock(&mtx);
}

void timer_start(uint32_t* timerId, void (*pCallback)(void), uint32_t timeout, bool periodic) {
    pthread_mutex_lock(&mtx);
    uint32_t timer_index;
    for (timer_index = 0; timer_index < MAX_TASK; timer_index++) {
        /* Find the empty slot where timer is not enabled */
        if (TimerTask[timer_index].timer_enable == false) {
            /* Enable the timer, with received parameters */
            TimerTask[timer_index].timer_enable = true;
            TimerTask[timer_index].reload_timeout_value = timeout;
            TimerTask[timer_index].curr_timeout_value = timeout;
            TimerTask[timer_index].periodic = periodic;
            TimerTask[timer_index].Callback = pCallback;
            /* Handler Id is used by caller to stop the timer */
            *timerId = timer_index;
            break;
        }
    }
    pthread_mutex_unlock(&mtx);
}

void timer_stop(uint32_t timerId) {
    pthread_mutex_lock(&mtx);
    if (TimerTask[timerId].timer_enable == false) {
        cout << "timer " << timerId << " is already stopped" << endl;
    } else if (timerId < MAX_TASK) {
        TimerTask[timerId].timer_enable = false;
        TimerTask[timerId].periodic = false;
        TimerTask[timerId].reload_timeout_value = 0xFFFFFFFF;
        TimerTask[timerId].curr_timeout_value = 0xFFFFFFFF;
        TimerTask[timerId].Callback = nullptr;
        cout << "timer " << timerId << " is stopped" << endl;
    }
    pthread_mutex_unlock(&mtx);
}

void* TimerInterruptThread(void* arg) {
    /*Generate Timer Interrupt every 1sec */
    static uint32_t counter = 0;
    while (1) {
        cout << counter++ << endl;
        TimerInterruptFunction();
        sleep(1);
    }
    return nullptr;
}

void* RunTasksThread(void* arg) {
    while (1) {
        pthread_mutex_lock(&mtx);
        while (!Task_queue.empty()) {
            uint32_t timer_index = Task_queue.front();
            Task_queue.pop();
            TimerTask[timer_index].Callback();
        }
        pthread_mutex_unlock(&mtx);
    }
    return nullptr;
}

void InitTimerTask() {
    uint32_t task_index;
    for (task_index = 0; task_index < MAX_TASK; task_index++) {
        timer_stop(task_index);
    }
}

int main() {
    pthread_t thread_id1;
    pthread_t thread_id2;
    InitTimerTask();
    pthread_create(&thread_id1, NULL, &TimerInterruptThread, NULL);
    pthread_create(&thread_id2, NULL, &RunTasksThread, NULL);

    uint32_t timerId1;
    cout << "Starting Periodic Timer 1 for every 2 sec" << endl;
    timer_start(&timerId1, Task1, 2, true);

    uint32_t timerId2;
    cout << "Starting Periodic Timer 2 for every 5 sec" << endl;

    timer_start(&timerId2, Task2, 5, true);

    uint32_t timerId3;
    cout << "Starting Oneshot Timer 3 in 15 sec" << endl;

    timer_start(&timerId3, Task3, 15, false);

    sleep(30);

    timer_stop(timerId1);
    timer_stop(timerId2);
    timer_stop(timerId3);

    pthread_cancel(thread_id1);
    pthread_cancel(thread_id2);

    return 0;
}
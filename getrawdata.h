#ifndef GETRAWDATA_H
#define GETRAWDATA_H

#include "qslidingwindow.h"

class GetRAWData
{
public:
    GetRAWData();
    ~GetRAWData();
    void bound(QSlidingWindow *sliding_window);
    QSlidingWindowConsume *append_user(int user_no);

private:
    QSlidingWindow *slidingwindow;
    int current_user;
};

#endif // GETRAWDATA_H

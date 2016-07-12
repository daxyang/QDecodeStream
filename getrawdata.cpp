#include "getrawdata.h"

GetRAWData::GetRAWData()
{
    current_user = 0;
}

GetRAWData::~GetRAWData()
{

}

void GetRAWData::bound(QSlidingWindow *sliding_window)
{
    slidingwindow = sliding_window;
}

QSlidingWindowConsume *GetRAWData::append_user(int user_no)
{
    if(current_user == 0)
    {
        current_user = user_no;
        int ok = slidingwindow->consume_linklist_append(current_user);
        if(ok > 0)
        {
            return slidingwindow->consume_linklist_getConsume(current_user);

        }
        else
        {
            current_user = 0;
            return NULL;
        }
    }
}


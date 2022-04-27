#include "onscreen_appcontroller.h"

KZPController::KZPController(QQuickItem *parent)
    : QQuickItem{parent}, mContainer{nullptr}
{

}

void KZPController::setContainer(QQuickItem* container)
{
    if(container != mContainer){
        if(mContainer){
            // transfer the sub item
        }
        mContainer = container;
        emit containerChanged(container);
    }
}

void return_to_origin() //中断函数 回到原点的程序
{
    if (digitalRead(SIG1) == 0)
    {
        // stepper->stopMove();
        // stepper->runForward();
        //stepper->forceStop();
        //stepper->setCurrentPosition(0);
        stepper->forceStopAndNewPosition(0); // 突然停止正在运行的步进器而不减速
        returnToOrigin_key = 0;
        if (returnToOrigin_loop == 0)
        {
            // 1-撞击原点后执行前进 2-前进一段距离后后退再次撞击原点
            returnToOrigin_loop = 1;
        }
        detachInterrupt(SIG1); // 取消中断
    }
}

void return_to_origin_loop() // 回到原点的程序-loop中执行
{
    //return_to_origin()触发时，将returnToOrigin_loop置1
    if (returnToOrigin_loop == 0) return;

    if (returnToOrigin_loop == 1) 
    {
        stepper->moveTo(eep_startPoint); // 前进到起点
        if (!stepper->isRunning())       // 前进完毕，再次回到原点
        {
            returnToOrigin_loop = 2; // 进入下一步
            attachInterrupt(SIG1, return_to_origin, FALLING);
            stepper->move(eep_startPoint * -2); // 移动
        }
    }
    // 再次回到原点结束，再次前进到起点
    else if (returnToOrigin_loop == 2 && !stepper->isRunning())
    {
        stepper->moveTo(eep_startPoint); // 前进到起点
        returnToOrigin_loop = 0;
    }
}

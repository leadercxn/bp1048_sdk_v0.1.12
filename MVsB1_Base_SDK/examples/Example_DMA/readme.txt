DMA_Example工程主要演示了 应用DMA的流程。

硬件环境要求:
    1. BP10xx系列开发板 ；
    2. 外接串口小板，TX/RX/GND 接至 A25/A24/GND 引脚 (波特率 256000)；

软件使用说明:
       上电启动之后，可通过串口输入不同指令，进入7种例程；
   1. 输入‘0’，进入DmaBlockMem2Mem;
   2. 输入‘1’，进入DmaBlockMem2Peri;
   3. 输入‘2’，进入DmaBlockPeri2Mem;
   4. 输入‘3’，进入DmaCirclePeri2Mem;
   5. 输入‘4’，进入DmaCircleMem2Peri;
   6. 输入‘5’，进入DmaTimerCircleMode;
   7. 输入‘6’，进入DmaTimerBlockMode;
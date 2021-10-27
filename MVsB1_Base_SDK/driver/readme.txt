Driver分为底层单独模块接口（Driver）以及一些通讯接口的API（DriverAPI）两部分

DriverAPI对Driver做了集成（一些通讯接口与DMA的集成）同时含有OS中的信号量，使得上层更方便的对通讯接口进行调用


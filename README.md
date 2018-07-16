# HP-625-Linux-Fancontrol
Fancontrol under Linux for HP-625 Notebooks for latest kernel versions.

## Notes on compiling/installing

If you want to use the fan control utility then you first have to compile it using cmake and g++. How you can install these programs depends on the linux distrubution you are using. Assuming that you are using debian or ubuntu you can issue the command 

> sudo apt install build-essential cmake git

to install the packages on your system.

Then clone the repository with git. Open a terminal window and then insert there 

> git clone https://github.com/TheAomx/HP-625-Linux-Fancontrol.git

Get into the cloned directory.

> cd HP-625-Linux-Fancontrol

and then use mkdir to create a build directory.

> mkdir build

Get into the build directory.

> cd build

 Then use cmake to generate the Makefile.

> cmake ..

After that use make to compile the program.

> make

You can use make install to copy the programm to the folder /opt if you want.

> sudo make install

Then you can run the program from the terminal now.

> sudo /opt/FanControl

Now your fans of your HP 625 should be controlled by the program. If you dont want to always manually start the program, you can use the included systemd service fancontrol.service to start the programm at the system startup. If you want to do that just issue the following commands in a terminal.

> cp ./fancontrol.service /etc/systemd/system/
> sudo systemctl enable fancontrol.service
> sudo systemctl start fancontrol.service

I hope this helps you with your issues installing my fan control program for the hp 625. 

# WimTiVoServer
Windows program to serve video files to TiVo devices

Requires FFmpeg executable either in the same directory as WimTiVoServer.exe or in findable via the PATH. https://www.ffmpeg.org/download.html#build-windows Because of the reliance on FFmpeg, any video format supported by FFmpeg can be transferred to a TiVo. Another benefit is that WimTiVoServer.exe doesn't need to be rebuilt or even reinstalled to support newer video formats, just replacing the ffmpeg executable with a newer version.

If run from the command line, will run for 12 hours and exit. If installed as a service runs continuously. 

The Program has three optional parameters:
 - -install (Intalls the program as a windows service to be run from its current location)
 - -remove (Removes the details of running as a service)
 - -ClearRegistry (Removes all registry references for the software)
 
The software pays attention to two primary registry keys.
 - HKEY_LOCAL_MACHINE\SOFTWARE\WimsWorld\WimTiVoServer\Container
 - HKEY_LOCAL_MACHINE\SOFTWARE\WimsWorld\WimTiVoServer\IgnoreExt
 
 Each is a ";" seperated list. The first is a pathspec of files to include and make available for a tivo on the local network to transfer. The second is a list of extensions to explicitly ignore. if the program is started and these do not exist, it will attempt to create them with default values.
 
 The program polls the container directories every 15 minutes for new files to include in the list.
 
# WimTiVoClient
Windows GUI Application that allows listing all of the files on your TiVo and transferring them to your computer. Optionally utilizes TiVoDecode and FFmpeg to transcode the retrieved .tivo file to a more friendly format. https://sourceforge.net/projects/tivodecode/ Needs the Media Access Key (MAK) from your TiVo to be able to communicate with the TiVo. Transferred file name format is based on the TiVo Desktop software that TiVo produced when they were a much more consumer friendly company.

Start the program, let it run for a minute or so and hopefully hear a beacon from your TiVo, exit the program, and restart it. Now you should hopefully be able to select your TiVo in the drop down list and then hit the Get Now Playing Button.

Important registry key that is visible but not modifiable from the interface, is where to store the recieved video files:
 - HKEY_CURRENT_USER\SOFTWARE\WimsWorld\WimTiVoClient\TiVo\TiVoFileDestination
 
# WimTiVoBeaconListener
Windows command line program that listens on UDP Port 2190 for TiVo Beacon messages and displays them. Useful to make sure that your machine is recieving network packets from your TiVo or the WimTiVoServer itself. If messages aren't being recieved, they may be blocked by a firewall.

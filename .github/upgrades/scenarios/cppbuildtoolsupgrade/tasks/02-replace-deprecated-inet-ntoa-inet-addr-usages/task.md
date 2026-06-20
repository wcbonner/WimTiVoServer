# 02-replace-deprecated-inet-ntoa-inet-addr-usages: Replace deprecated inet_ntoa / inet_addr usages

   - Files to inspect/edit:
	 - C:\Users\Wim\Source\Repos\WimTiVoServer\WimTiVoServer.cpp (lines ~988, ~1804-1812)
	 - C:\Users\Wim\Source\Repos\WimTiVoServer\WimTiVoTestCode.cpp (line ~661)
   - Changes:
	 - Add helper string InetAddrToString(const in_addr& addr) using InetNtop (or InetNtopA) and buffer size INET_ADDRSTRLEN
	 - Include Winsock2.h and Ws2tcpip.h where needed (ensure ordering)
	 - Replace inet_ntoa/ipAddr usage with helper
	 - If project missing Ws2_32.lib linkage, update project to link it (vcxproj edit; will unload project before editing and validate per rules)
   - Done when: Project compiles and C4996 warnings for inet_* are removed.

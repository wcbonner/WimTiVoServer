# 01-prepare-and-add-conversion-helper: Prepare and add conversion helper

   - Files to inspect/edit:
	 - C:\Users\Wim\Source\Repos\WimTiVoServer\WimTiVoServer.cpp (add helper near top or in an internal utility section)
   - Changes:
	 - Add a Utf8ToWString helper using MultiByteToWideChar
	 - Replace std::wstring_convert calls at lines ~828 and ~1947-1949 with helper calls
	 - Add necessary includes (#include <Windows.h>, <string>) and guard with #ifdef _WIN32 if appropriate
   - Done when: Project compiles and the STL4017/C4996 codecvt warnings are removed for those replacements.

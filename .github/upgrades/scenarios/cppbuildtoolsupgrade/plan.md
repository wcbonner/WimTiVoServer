# Plan: Fix in-scope issues from MSVC Build Tools retargeting

Generated: Planning stage for C++ Build Tools Upgrade (Option A — fix in-scope items only)
Solution: C:\Users\Wim\Source\Repos\WimTiVoServer\WimTiVoServer.sln

## Goal
Address issues classified as in-scope in assessment.md:
- Replace uses of std::wstring_convert / <codecvt> in WimTiVoServer.cpp (STL4017 / C4996 deprecation warnings)
- Replace deprecated Win32 socket APIs (inet_ntoa / inet_addr) with InetNtop/InetPton (C4996 warnings)

Scope: limited to the files and warnings listed above. We will not attempt to remediate numeric narrowing (C4244/C4267) or other CRT "unsafe" warnings in this plan.

## Investigation summary (findings)
- std::wstring_convert<std::codecvt_utf8<wchar_t>> is used in WimTiVoServer.cpp at lines ~828 and ~1947-1949 to convert UTF-8 std::string members (myServer.m_swversion, myServer.m_platform, myServer.m_identity) into std::wstring.
- inet_ntoa / inet_addr are used in WimTiVoServer.cpp (lines ~992, ~1809, etc.) and in WimTiVoTestCode.cpp. These functions are deprecated; InetNtop/InetPton (or their ANSI/Unicode variants) are the recommended replacements and are available when including <Ws2tcpip.h> and linking Ws2_32.lib. Winsock2.h must be included before Windows.h.

## Proposed approach
We will apply targeted, minimal code changes that remove the deprecation warnings and keep behavior unchanged:

1) Replace std::wstring_convert usage with a small UTF-8-to-wstring helper using MultiByteToWideChar (Windows API). This avoids adding large platform-agnostic conversions and uses well-supported Windows APIs.
   - Create an inline helper in an existing common source file (prefer: WimTiVoServer.cpp or add a new helper in a small utils file) with signature: std::wstring Utf8ToWString(const std::string& utf8).
   - Replace each use of std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(...) with Utf8ToWString(...).
   - Add #include <Windows.h> and necessary headers where helper is defined. Ensure helper is only compiled for Windows builds.

Trade-offs: Using MultiByteToWideChar ties conversion to Windows APIs (acceptable for this Windows-only project). Alternative would be to keep codecvt and silence the deprecation with _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING — less invasive but leaves deprecated API usage.

2) Replace inet_ntoa / inet_addr usages with InetNtop / InetPton (or InetNtopA/InetPtonA) calls:
   - Include <Winsock2.h> and <Ws2tcpip.h> in source files that call these functions. Ensure Winsock2.h is included before Windows.h if present.
   - Replace expressions like inet_ntoa(ipAddr) with a small helper that calls InetNtop(AF_INET, &ipAddr, buf, INET_ADDRSTRLEN) and returns a std::string or appends buffer content to stringstreams.
   - For places that pass result directly to streams (e.g., ss << inet_ntoa(ipAddr)), change to use buffer and append as string.
   - Verify Ws2_32.lib is linked (project likely already uses Winsock; if not, add linkage in project file). We will inspect the project include/link settings before editing .cpp files and add project edits only if necessary.

Trade-offs: InetNtop/InetPton are newer and reentrant; require linking Ws2_32.lib and windows SDK support (available on current dev machine per preconditions). If you prefer minimal change, we can instead define _WINSOCK_DEPRECATED_NO_WARNINGS (suppresses warnings) — but this does not modernize APIs.

## Tasks (ordered by build dependency)

1. Task 01 — Prepare and add conversion helper
   - Files to inspect/edit:
	 - C:\Users\Wim\Source\Repos\WimTiVoServer\WimTiVoServer.cpp (add helper near top or in an internal utility section)
   - Changes:
	 - Add a Utf8ToWString helper using MultiByteToWideChar
	 - Replace std::wstring_convert calls at lines ~828 and ~1947-1949 with helper calls
	 - Add necessary includes (#include <Windows.h>, <string>) and guard with #ifdef _WIN32 if appropriate
   - Done when: Project compiles and the STL4017/C4996 codecvt warnings are removed for those replacements.

2. Task 02 — Replace deprecated inet_ntoa / inet_addr usages
   - Files to inspect/edit:
	 - C:\Users\Wim\Source\Repos\WimTiVoServer\WimTiVoServer.cpp (lines ~988, ~1804-1812)
	 - C:\Users\Wim\Source\Repos\WimTiVoServer\WimTiVoTestCode.cpp (line ~661)
   - Changes:
	 - Add helper string InetAddrToString(const in_addr& addr) using InetNtop (or InetNtopA) and buffer size INET_ADDRSTRLEN
	 - Include Winsock2.h and Ws2tcpip.h where needed (ensure ordering)
	 - Replace inet_ntoa/ipAddr usage with helper
	 - If project missing Ws2_32.lib linkage, update project to link it (vcxproj edit; will unload project before editing and validate per rules)
   - Done when: Project compiles and C4996 warnings for inet_* are removed.

3. Task 03 — Verify and finalize
   - Actions:
	 - Run incremental build (cppupgrade_build_and_get_issues) after Task 01 and after Task 02 to verify no regressions
	 - Run full rebuild (cppupgrade_rebuild_and_get_issues) after both changes complete
	 - Address any immediate compilation errors introduced by replacements
   - Done when: Final rebuild shows no new warnings in the changed areas (we accept other pre-existing warnings remain).

## Risks & Open Questions
- I will make per-file edits and may need to unload/reload vcxproj if project file edits are required to add library linkage. Confirm you permit these edits on the working branch CppBuildToolsUpgrade-master.
- If Winsock headers are missing or projects use different include ordering, minor project adjustments may be required — I will update the plan if so.
- If you prefer suppression (macro) instead of code replacement for codecvt/inet_* warnings, tell me now.

## Estimated steps I will take after you approve
1. Start Task 01 (edit WimTiVoServer.cpp, build)
2. Start Task 02 (edit files listed, build)
3. Run final rebuild and report results


---
Artifacts to be created in this stage:
- plan.md: C:\Users\Wim\Source\Repos\WimTiVoServer\.github\upgrades\scenarios\cppbuildtoolsupgrade\plan.md

Reply with: "approve" to proceed to Execution, or "change" and describe changes (e.g., prefer suppression macros).
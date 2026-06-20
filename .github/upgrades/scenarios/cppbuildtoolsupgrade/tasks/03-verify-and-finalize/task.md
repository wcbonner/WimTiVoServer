# 03-verify-and-finalize: Verify and finalize

   - Actions:
	 - Run incremental build (cppupgrade_build_and_get_issues) after Task 01 and after Task 02 to verify no regressions
	 - Run full rebuild (cppupgrade_rebuild_and_get_issues) after both changes complete
	 - Address any immediate compilation errors introduced by replacements
   - Done when: Final rebuild shows no new warnings in the changed areas (we accept other pre-existing warnings remain).

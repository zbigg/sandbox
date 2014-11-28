struct


mib::fragment tinfra("tinfra");
mib::fragment tinfra_test(tinfra_test, "test");
mib::fragment tinfra_test_rusage(tinfra_test, "rusage");

mib::leaf<int> tinfra_test_rusage_user_cpu_seconds(tinfra_test_rusage, 1, "user_cpu_seconds");
mib::leaf<int> tinfra_test_rusage_user_cpu_microseconds(tinfra_test_rusage, 2, "user_cpu_microseconds");
mib::leaf<int> tinfra_test_rusage_user_cpu_time(tinfra_test_rusage, 3, "user_cpu_time");

mib::leaf<int> tinfra_test_rusage_system_cpu_seconds(tinfra_test_rusage, 4, "system_cpu_seconds");
mib::leaf<int> tinfra_test_rusage_system_cpu_microseconds(tinfra_test_rusage, 5, "system_cpu_microseconds");
mib::leaf<int> tinfra_test_rusage_system_cpu_time(tinfra_test_rusage, 6, "system_cpu_time");

mib::fragment  tinfra_test_coverage(tinfra_test, "coverage");
mib::leaf<int> tinfra_test_coverage_branches_total(tinfra_test_coverage, 1, "branches_total");
mib::leaf<int> tinfra_test_coverage_branches_taken(tinfra_test_coverage, 2, "branches_taken");
mib::leaf<int> tinfra_test_coverage_branches(tinfra_test_coverage, 3, "branches");

mib::leaf<int> tinfra_test_coverage_lines_total(tinfra_test_coverage, 4, "branches_total");
mib::leaf<int> tinfra_test_coverage_lines_taken(tinfra_test_coverage, 5, "branches_taken");
mib::leaf<int> tinfra_test_coverage_lines(tinfra_test_coverage, 6, "lines");

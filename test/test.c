#include <glib.h>
#include "utils/utils.h"

void test_issue_19() {
	g_test_bug("19");
	g_assert_cmpstr("WINDOWS-1256", ==, get_encoding_by_lcid(0xc01));
}

int main(int argc, char* argv[]) {
	g_test_init(&argc, &argv, NULL);
	g_test_bug_base("http://code.google.com/p/chmsee/issues/detail?id=");
	g_test_add_func("/chmsee/19", test_issue_19);
	g_test_run();
	return 0;
}

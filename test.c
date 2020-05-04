#include <stdio.h>
#include <stdlib.h>
#include "src/so_scheduler.h"

#define SO_MAX_UNITS 32

/*
 * 15) Test exec priority
 *
 * tests if the scheduler properly preempts a task according to priorities
 */
static unsigned int test_exec_last_priority;
static unsigned int test_exec_status;

#define SO_TEST_FAIL 0
#define SO_TEST_SUCCESS 1

#define so_error(msg, ...) fprintf(stderr, "ERR: " msg "\n", ##__VA_ARGS__)

#define so_fail(msg) \
	do { \
		so_error(msg); \
		exit(-1); \
	} while (0)

/* fails if the last priority set is not prio */
#define SO_FAIL_IF_NOT_PRIO(prio, msg) \
	do { \
		if ((prio) != test_exec_last_priority) { \
			test_exec_status = SO_TEST_FAIL; \
			so_error("\t\tEXPECTED PRIORITY: %d\t|\tGOT LAST PRIORITY: %d\n", prio, test_exec_last_priority);\
			so_fail(msg); \
		} \
		test_exec_last_priority = priority; \
	} while (0)
#ifdef PRI
/*
 * Note: P1 means tasks with priority 1
 *
 * Scenario:
 * - P2 spawns P4
 * - P4 spawns P3
 * - P3 spawns P1
 */
static void test_sched_handler_15(unsigned int priority)
{
	switch (priority) {
	case 1:
		/* test if I was scheduled before P2 */
		SO_FAIL_IF_NOT_PRIO(2,
			"scheduled a test with a bogus priority");
		break;

	case 2:
		/* fork P4 */
		so_fork(test_sched_handler_15, 4);

		/* if I was not preempted or P3 didn't run - error */
		SO_FAIL_IF_NOT_PRIO(3, "task 2 was not preempted");
		break;

	case 3:
		/* test if someobdy else run except P4 */
		SO_FAIL_IF_NOT_PRIO(4, "highest priority was not scheduled");
		so_fork(test_sched_handler_15, 1);

		/* P1 < P3 - I still have to run */
		SO_FAIL_IF_NOT_PRIO(3,
			"somebody else was scheduled instead of task 3");
		break;

	case 4:
		test_exec_last_priority = 4;

		/* fork lower priority P3 */
		so_fork(test_sched_handler_15, 3);

		/* I shouldn't have been preempted */
		SO_FAIL_IF_NOT_PRIO(4,
			"somebody else was scheduled instead of task 4");
		break;
	}

	so_exec();
}

int main(void)
{
	test_exec_status = SO_TEST_SUCCESS;
	so_init(1, 0);

	test_exec_last_priority = 2;
	so_fork(test_sched_handler_15, 2);

	sched_yield();
	so_end();

	if (test_exec_status&& test_exec_last_priority == 1) {
		fprintf(stderr, "BINES\n");
	} else {
		fprintf(stderr, "NASOL\n");
	}

	return 0;
}
#endif
static tid_t test_exec_last_tid;
static tid_t test_tid_13_1;
static tid_t test_tid_13_2;

static inline int equal_tids(tid_t t1, tid_t t2)
{
	return t1 == t2;
}
static inline tid_t get_tid(void)
{
	return pthread_self();
}

#define SO_TEST_AND_SET(expect_id, new_id) \
	do { \
		if (equal_tids((expect_id), INVALID_TID) || \
				equal_tids((new_id), INVALID_TID)) \
			so_fail("invalid task id"); \
		if (!equal_tids(test_exec_last_tid, (expect_id))) {\
			so_error("expexted_id = %ld \t last_exec_id = %ld\n", expect_id, test_exec_last_tid);\
			so_fail("invalid tasks order"); \
		}\
		test_exec_last_tid = (new_id); \
	} while (0)

#ifdef PREEMPT


static void test_sched_handler_13_2(unsigned int dummy)
{
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_2);
	so_exec();
	test_exec_status = SO_TEST_SUCCESS;
}

static void test_sched_handler_13_1(unsigned int dummy)
{
	test_exec_last_tid = test_tid_13_1 = get_tid();
	test_tid_13_2 = so_fork(test_sched_handler_13_2, 0);

	/* allow the other thread to init */
	sched_yield();

	/* I should continue running */
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_1, test_tid_13_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_13_2, test_tid_13_1);
	so_exec();

	/* make sure nobody changed it until now */
	test_exec_status = SO_TEST_FAIL;
}

int main(void)
{
	test_exec_status = SO_TEST_FAIL;

	/* quantum is 2, so each task should be preempted
	 * after running two instructions
	 */
	so_init(2, 0);

	so_fork(test_sched_handler_13_1, 0);

	sched_yield();
	so_end();

	if (test_exec_status == SO_TEST_FAIL) {
		fprintf(stderr, "NASOL\n");
	} else {
		fprintf(stderr, "BINES\n");
	}

	return 0;
}
#endif
#ifdef MULTIPLE
static tid_t test_tid_14_1;
static tid_t test_tid_14_2;
static tid_t test_tid_14_3;
static tid_t test_tid_14_4;

static void test_sched_handler_14_4(unsigned int dummy)
{
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_4);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_4, test_tid_14_4);

	test_exec_status = SO_TEST_FAIL;
}

static void test_sched_handler_14_3(unsigned int dummy)
{
	SO_TEST_AND_SET(test_tid_14_2, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);
	so_exec();
	/* should be preempted here */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_3);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_3, test_tid_14_3);

	/* done scheduling */
	test_exec_status = SO_TEST_SUCCESS;
}

static void test_sched_handler_14_2(unsigned int dummy)
{
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_2, test_tid_14_2);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_2, test_tid_14_2);

	/* leaving - thread 3 should start */
	test_exec_status = SO_TEST_FAIL;
}

static void test_sched_handler_14_1(unsigned int dummy)
{
	test_exec_last_tid = test_tid_14_1 = get_tid();
	test_tid_14_2 = so_fork(test_sched_handler_14_2, 0);
	/* allow the other thread to init */
	sched_yield();

	/* I should continue running */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);
	test_tid_14_3 = so_fork(test_sched_handler_14_3, 0);
	sched_yield();

	/* I should continue running */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);
	test_tid_14_4 = so_fork(test_sched_handler_14_4, 0);
	sched_yield();

	/* still me */
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);
	so_exec();

	/* should be preempted here */
	SO_TEST_AND_SET(test_tid_14_4, test_tid_14_1);
	so_exec();
	SO_TEST_AND_SET(test_tid_14_1, test_tid_14_1);

	/* leaving - make sure the others execute successfully */
	test_exec_status = SO_TEST_FAIL;
}

int main(void)
{
	test_exec_status = SO_TEST_FAIL;

	so_init(4, 0);

	so_fork(test_sched_handler_14_1, 0);

	sched_yield();
	so_end();

	if (test_exec_status == SO_TEST_FAIL)
		fprintf(stderr, "NASOL\n");
	else
		fprintf(stderr, "BINES\n");

	return 0;
}
#endif

#ifdef FORK
/*
 * 11) Test multiple fork thread ids
 *
 * tests if the scheduler runs each fork with a different id
 */
static unsigned int test_fork_rand_tests;
static unsigned int test_fork_execution_status;
static tid_t test_fork_exec_tids[SO_MAX_UNITS];
static tid_t test_fork_tids[SO_MAX_UNITS];

static void test_sched_handler_11_worker(unsigned int dummy)
{
	static unsigned int exec_idx;

	/* signal that he's the one that executes in round exec_idx */
	test_fork_exec_tids[exec_idx++] = get_tid();
	test_fork_execution_status = SO_TEST_SUCCESS;
}

static void test_sched_handler_11_master(unsigned int dummy)
{
	unsigned int i;

	/*
	 * this thread should not be preempted as it executes maximum
	 * SO_MAX_UNITS - 1, and the quantum time is SO_MAX_UNITS
	 */
	/* use a cannary value to detect overflow */
	test_fork_tids[test_fork_rand_tests] = get_tid();
	test_fork_exec_tids[test_fork_rand_tests] = get_tid();
	for (i = 0; i < test_fork_rand_tests; i++)
		test_fork_tids[i] = so_fork(test_sched_handler_11_worker, 0);
}

int main(void)
{
	unsigned int i;

	// test_fork_rand_tests = get_rand(1, SO_MAX_UNITS - 1);
	test_fork_rand_tests = SO_MAX_UNITS - 3;
	test_fork_execution_status = SO_TEST_FAIL;

	so_init(SO_MAX_UNITS, 0);

	so_fork(test_sched_handler_11_master, 0);

	sched_yield();
	so_end();

	if (test_fork_execution_status == SO_TEST_SUCCESS) {
		/* check threads order */
		for (i = 0; i <= test_fork_rand_tests; i++) {
			if (!equal_tids(test_fork_exec_tids[i],
				test_fork_tids[i])) {
				so_error("different thread ids");
				test_fork_execution_status = SO_TEST_FAIL;
				break;
			}
		}
	} else {
		so_error("threads execution failed");
	}

	if (test_fork_execution_status == SO_TEST_SUCCESS)
		fprintf(stderr, "BINES\n");
	else
		fprintf(stderr, "NASOL\n");
}
#endif

#define get_rand(min, max) ((rand() % (max - min)) + min)

#ifdef IO
#define SO_DEV0		0
#define SO_DEV1		1
#define SO_DEV2		2
#define SO_DEV3		3

#define SO_PREEMPT_UNITS	3

static unsigned int exec_time;
static unsigned int exec_devs;
static unsigned int last_priority;
static unsigned int exec_priority;
static unsigned int test_exec_status = SO_TEST_FAIL;

/*
 * 21) Test priorities and IO
 *
 * tests if the scheduler properly handles IO devices and preemption
 */
/* fails if the last priority set is not prio */
#define FAIL_IF_NOT_PRIO(prio, msg) \
	do { \
		if ((prio) != last_priority) { \
			test_exec_status = SO_TEST_FAIL; \
			so_error("Se astepta threadul cu PRIO %u dar a fost thread-ul cu PRIO %u\n", prio, last_priority);\
			so_fail(msg); \
		} \
		last_priority = priority; \
	} while (0)

/*
 * Threads are mixed to wait/signal lower/higher priorities
 * P2 refers to the task with priority 2
 */
static void test_sched_handler_21(unsigned int priority)
{
	switch (priority) {
	case 1:
		/* P2 should be the one that executed last */
		FAIL_IF_NOT_PRIO(2, "should have been woke by P2");
		if (so_signal(SO_DEV3) == 0)
			so_fail("dev3 does not exist");
		so_exec();
		FAIL_IF_NOT_PRIO(1, "invalid preemption");
		if (so_signal(SO_DEV0) != 2)
			so_fail("P1 should wake P3 and P4 (dev0)");
		FAIL_IF_NOT_PRIO(2, "preempted too early");
		if (so_signal(SO_DEV1) != 1)
			so_fail("P1 should wake P3 (dev1)");
		FAIL_IF_NOT_PRIO(2, "woke by someone else");
		if (so_signal(SO_DEV0) != 1)
			so_fail("P1 should wake P4 (dev0)");
		FAIL_IF_NOT_PRIO(4, "should be the last one");
		so_exec();
		FAIL_IF_NOT_PRIO(1, "someone else was running");
		break;

	case 2:
		last_priority = 2;
		/* wait for dev 3 - invalid device */
		if (so_wait(SO_DEV3) == 0)
			so_fail("dev3 does not exist");
		/* spawn all the tasks */
		so_fork(test_sched_handler_21, 4);
		so_fork(test_sched_handler_21, 3);
		so_fork(test_sched_handler_21, 1);
		so_exec();
		so_exec();

		/* no one should have ran until now */
		FAIL_IF_NOT_PRIO(2, "somebody else ran before P2");
		if (so_wait(SO_DEV1) != 0)
			so_fail("cannot wait on dev1");
		FAIL_IF_NOT_PRIO(3, "should run after P3");
		if (so_wait(SO_DEV2) != 0)
			so_fail("cannot wait on dev2");
		FAIL_IF_NOT_PRIO(3, "only P3 could wake me");
		so_exec();
		break;

	case 3:
		if (so_wait(SO_DEV0) != 0)
			so_fail("P3 cannot wait on dev0");
		FAIL_IF_NOT_PRIO(4, "priority order violated");
		if (so_wait(SO_DEV1) != 0)
			so_fail("P3 cannot wait on dev1");
		FAIL_IF_NOT_PRIO(1, "someone else woke P3");
		if (so_signal(SO_DEV2) != 1)
			so_fail("P3 should wake P2 (dev2)");
		break;

	case 4:
		if (so_wait(SO_DEV0) != 0)
			so_fail("P4 cannot wait on dev0");
		FAIL_IF_NOT_PRIO(1, "lower priority violation");
		if (so_signal(SO_DEV1) != 1)
			so_fail("P4 should wake P2 (dev1)");
		if (so_wait(SO_DEV0) != 0)
			so_fail("P4 cannot wait on dev0");
		FAIL_IF_NOT_PRIO(1, "someone else woke dev0");
		break;
	}
}

/* tests the IO and priorities */
int main(void)
{
	test_exec_status = SO_TEST_SUCCESS;

	so_init(1, 3);

	so_fork(test_sched_handler_21, 2);

	sched_yield();
	so_end();

	if (test_exec_status == SO_TEST_SUCCESS && last_priority == 1)
		fprintf(stderr, "BINES\n");
	else
		fprintf(stderr, "NASOL\n");

	return 0;
}
#endif
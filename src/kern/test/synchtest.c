/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Synchronization test code.
 */

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>

#define NSEMLOOPS     63
#define NLOCKLOOPS    120
#define NCVLOOPS      5
#define NTHREADS      32

//차량의 접근하는 위치 북-동-남-서
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

// 차량 직진, 우회전, 좌회전
#define STRAIGHT 0
#define TURNRIGHT 1
#define TURNLEFT 2
#define DIRECTION 3

static volatile unsigned long testval1;
static volatile unsigned long testval2;
static volatile unsigned long testval3;
static struct semaphore *testsem;
static struct lock *testlock;
static struct cv *testcv;
static struct semaphore *donesem;

//세마포어
static struct semaphore* sem_NW;
static struct semaphore* sem_NE;
static struct semaphore* sem_SW;
static struct semaphore* sem_SE;
static struct semaphore* sem_print;
static struct semaphore* sem_thread;
// 좌회전 lock용
static struct lock* acquire_lock; 


//인쇄용으로 제공된 메시지 기능 함수
static
void
message_function(const char* from, const char* to, int car_number, int status) {
	P(sem_print);
	
	if (status == STRAIGHT) {
		kprintf("car %d is coming from %s going STRAIGHT\n", car_number, from);
		kprintf("car = %d, in = %s, out = %s\n", car_number, from, to);
	}
	else if (status == TURNRIGHT) {
		kprintf("car %d is coming from %s going TURNRIGHT\n", car_number, from);
		kprintf("car = %d, in = %s, out = %s\n", car_number, from, to);
	}
	else if (status == TURNLEFT) {
		kprintf("car %d is coming from %s going TURNLEFT\n", car_number, from);
		kprintf("car = %d, in = %s, out = %s\n", car_number, from, to);
	}
	V(sem_print);
}

//세마포어 초기화
static
void
inititems(void)
{
	if (sem_thread == NULL) {
		sem_thread = sem_create("sem_thread", 0);
		if (sem_thread == NULL) {
			kprintf("sem_thread error");
		}
	}
	if (sem_print == NULL) {
		sem_print = sem_create("sem_print", 1);
		if (sem_print == NULL) {
			kprintf("sem_print error");
		}
	}

	if (sem_NW == NULL) {
		sem_NW = sem_create("NW", 1);
		if (sem_NW == NULL) {
			panic("NW Error");
		}
	}
	if (sem_NE == NULL) {
		sem_NE = sem_create("NE", 1);
		if (sem_NE == NULL) {
			panic("NE Error");
		}
	}
	if (sem_SE == NULL) {
		sem_SE = sem_create("SE", 1);
		if (sem_SE == NULL) {
			panic("SE Error");
		}
	}
	if (sem_SW == NULL) {
		sem_SW = sem_create("SW", 1);
		if (sem_SW == NULL) {
			panic("SW Error");
		}
	}
	if (testsem == NULL) {
		testsem = sem_create("testsem", 2);
		if (testsem == NULL) {
			panic("synchtest: sem_create failed\n");
		}
	}
	if (testlock == NULL) {
		testlock = lock_create("testlock");
		if (testlock == NULL) {
			panic("synchtest: lock_create failed\n");
		}
	}
	if (testcv == NULL) {
		testcv = cv_create("testlock");
		if (testcv == NULL) {
			panic("synchtest: cv_create failed\n");
		}
	}
	if (donesem == NULL) {
		donesem = sem_create("donesem", 0);
		if (donesem == NULL) {
			panic("synchtest: sem_create failed\n");
		}
	}
	if (acquire_lock == NULL) {
		acquire_lock = lock_create("acquire_lock");
		if (acquire_lock == NULL) {
			panic("acquire_lock creation failed");
		}
	}

}



//직진
static void straight(int car_input, int car_number) {
	// N 방향으로 들어와서 직진
	// 우선 순위 : SW -> NW
	if (car_input == NORTH) {
		P(sem_SW);
		P(sem_NW);
		message_function("NORTH", "SOUTH", car_number, STRAIGHT);
		V(sem_NW);
		V(sem_SW);
	}

	// E 방향으로 들어와서 직진
	// 우선 순위 : NE -> NW
	else if (car_input == EAST) {
		P(sem_NE);
		P(sem_NW);
		message_function("EAST", "WEST", car_number, STRAIGHT);
		V(sem_NW);
		V(sem_NE);
	}

	// S 방향으로 들어와서 직진
	// 우선 순위 : NE -> SE
	else if (car_input == SOUTH) {
		P(sem_NE);
		P(sem_SE);
		message_function("SOUTH", "NORTH", car_number, STRAIGHT);
		V(sem_SE);
		V(sem_NE);
	}

	// W 방향으로 들어와서 직진
	// 우선 순위 : SW -> SE
	else if (car_input == WEST) {
		P(sem_SW);
		P(sem_SE);
		message_function("WEST", "EAST", car_number, STRAIGHT);
		V(sem_SE);
		V(sem_SW);
	}
}

//우회전
static void right(int car_input, int car_number) {
	// N 방향으로 들어와서 우회전: NW
	if (car_input == NORTH) {
		P(sem_NW);
		message_function("NORTH", "WEST", car_number, TURNRIGHT);
		V(sem_NW);
	}
	// E 방향으로 들어와서 우회전: NE
	else if (car_input == EAST) {
		P(sem_NE);
		message_function("EAST", "NORTH", car_number, TURNRIGHT);
		V(sem_NE);
	}
	// S 방향으로 들어와서 우회전: SE
	else if (car_input == SOUTH) {
		P(sem_SE);
		message_function("SOUTH", "EAST", car_number, TURNRIGHT);
		V(sem_SE);
	}
	// W 방향으로 들어와서 우회전: SW
	else if (car_input == WEST) {
		P(sem_SW);
		message_function("WEST", "SOUTH", car_number, TURNRIGHT);
		V(sem_SW);
	}
}

//좌회전
static void left(int car_input, int car_number) {
	// N 방향으로 들어와서 좌회전: SW -> NW -> SE
	if (car_input == NORTH) {
		
		lock_acquire(acquire_lock);
		P(sem_SW);
		P(sem_NW);
		P(sem_SE);
		lock_release(acquire_lock);
		message_function("NORTH", "EAST", car_number, TURNLEFT);
		V(sem_SE);
		V(sem_NW);
		V(sem_SW);
	}
	// S 방향으로 들어와서 좌회전: NE -> NW -> SE
	else if (car_input == SOUTH) {
		lock_acquire(acquire_lock);
		P(sem_NE);
		P(sem_NW);
		P(sem_SE);
		lock_release(acquire_lock);
		message_function("SOUTH", "WEST", car_number, TURNLEFT);
		V(sem_SE);
		V(sem_NW);
		V(sem_NE);
	}
	// E 방향으로 들어와서 좌회전: SW -> NE -> NW
	else if (car_input == EAST) {
		lock_acquire(acquire_lock);
		P(sem_SW);
		P(sem_NE);
		P(sem_NW);
		lock_release(acquire_lock);
		message_function("EAST", "SOUTH", car_number, TURNLEFT);
		V(sem_NW);
		V(sem_NE);
		V(sem_SW);
	}
	// W 방향으로 들어와서 좌회전: SW -> NE -> SE
	else if (car_input == WEST) {
		lock_acquire(acquire_lock);
		P(sem_SW);
		P(sem_NE);
		P(sem_SE);
		lock_release(acquire_lock);
		message_function("WEST", "NORTH", car_number, TURNLEFT);
		V(sem_SE);
		V(sem_NE);
		V(sem_SW);
	}
}



static
void
semtestthread(void *junk, unsigned long num)
{

	(void)junk;

	int car_input = random() % 4;
	int direction = random() % 3;


	if (direction == STRAIGHT) {
		message_function("STRAIGHT", "", num, DIRECTION);
		straight(car_input, num);
	}
	else if (direction == TURNLEFT) {
		message_function("TURNLEFT", "", num, DIRECTION);
		left(car_input, num);
	}
	else if (direction == TURNRIGHT) {
		message_function("TURNRIGHT", "", num, DIRECTION);
		right(car_input, num);
	}


	/*
	 * Only one of these should print at a time.
	 */
	
	P(testsem);

	/*
	kprintf("Thread %2lu: ", num);

	char ch = 'A' + (num % 26);
	for (i = 0; i < NSEMLOOPS; i++) {
		kprintf("%c", ch);
	}
	kprintf("\n");
	*/
	V(donesem);
		
}

int
semtest(int nargs, char **args)
{
	int i, result;

	(void)nargs;
	(void)args;

	inititems();
	kprintf("Starting semaphore test...\n");
	kprintf("If this hangs, it's broken: ");
	P(testsem);
	P(testsem);
	kprintf("ok\n");

	for (i=0; i<NTHREADS; i++) {
		result = thread_fork("semtest", NULL, semtestthread, NULL, i);
		if (result) {
			panic("semtest: thread_fork failed: %s\n",
			      strerror(result));
		}
	}

	for (i=0; i<NTHREADS; i++) {
		V(testsem);
		P(donesem);
	}

	/* so we can run it again */
	V(testsem);
	V(testsem);

	kprintf("Semaphore test done.\n");
	return 0;
}

static
void
fail(unsigned long num, const char *msg)
{
	kprintf("thread %lu: Mismatch on %s\n", num, msg);
	kprintf("Test failed\n");

	lock_release(testlock);

	V(donesem);
	thread_exit();
}

static
void
locktestthread(void *junk, unsigned long num)
{
	int i;
	(void)junk;

	for (i=0; i<NLOCKLOOPS; i++) {
		lock_acquire(testlock);
		testval1 = num;
		testval2 = num*num;
		testval3 = num%3;

		if (testval2 != testval1*testval1) {
			fail(num, "testval2/testval1");
		}

		if (testval2%3 != (testval3*testval3)%3) {
			fail(num, "testval2/testval3");
		}

		if (testval3 != testval1%3) {
			fail(num, "testval3/testval1");
		}

		if (testval1 != num) {
			fail(num, "testval1/num");
		}

		if (testval2 != num*num) {
			fail(num, "testval2/num");
		}

		if (testval3 != num%3) {
			fail(num, "testval3/num");
		}

		lock_release(testlock);
	}
	V(donesem);
}


int
locktest(int nargs, char **args)
{
	int i, result;

	(void)nargs;
	(void)args;

	inititems();
	kprintf("Starting lock test...\n");

	for (i=0; i<NTHREADS; i++) {
		result = thread_fork("synchtest", NULL, locktestthread,
				     NULL, i);
		if (result) {
			panic("locktest: thread_fork failed: %s\n",
			      strerror(result));
		}
	}
	for (i=0; i<NTHREADS; i++) {
		P(donesem);
	}

	kprintf("Lock test done.\n");

	return 0;
}

static
void
cvtestthread(void *junk, unsigned long num)
{
	int i;
	volatile int j;
	struct timespec ts1, ts2;

	(void)junk;

	for (i=0; i<NCVLOOPS; i++) {
		lock_acquire(testlock);
		while (testval1 != num) {
			gettime(&ts1);
			cv_wait(testcv, testlock);
			gettime(&ts2);

			/* ts2 -= ts1 */
			timespec_sub(&ts2, &ts1, &ts2);

			/* Require at least 2000 cpu cycles (we're 25mhz) */
			if (ts2.tv_sec == 0 && ts2.tv_nsec < 40*2000) {
				kprintf("cv_wait took only %u ns\n",
					ts2.tv_nsec);
				kprintf("That's too fast... you must be "
					"busy-looping\n");
				V(donesem);
				thread_exit();
			}

		}
		kprintf("Thread %lu\n", num);
		testval1 = (testval1 + NTHREADS - 1)%NTHREADS;

		/*
		 * loop a little while to make sure we can measure the
		 * time waiting on the cv.
		 */
		for (j=0; j<3000; j++);

		cv_broadcast(testcv, testlock);
		lock_release(testlock);
	}
	V(donesem);
}

int
cvtest(int nargs, char **args)
{

	int i, result;

	(void)nargs;
	(void)args;

	inititems();
	kprintf("Starting CV test...\n");
	kprintf("Threads should print out in reverse order.\n");

	testval1 = NTHREADS-1;

	for (i=0; i<NTHREADS; i++) {
		result = thread_fork("synchtest", NULL, cvtestthread, NULL, i);
		if (result) {
			panic("cvtest: thread_fork failed: %s\n",
			      strerror(result));
		}
	}
	for (i=0; i<NTHREADS; i++) {
		P(donesem);
	}

	kprintf("CV test done\n");

	return 0;
}

////////////////////////////////////////////////////////////

/*
 * Try to find out if going to sleep is really atomic.
 *
 * What we'll do is rotate through NCVS lock/CV pairs, with one thread
 * sleeping and the other waking it up. If we miss a wakeup, the sleep
 * thread won't go around enough times.
 */

#define NCVS 250
#define NLOOPS 40
static struct cv *testcvs[NCVS];
static struct lock *testlocks[NCVS];
static struct semaphore *gatesem;
static struct semaphore *exitsem;

static
void
sleepthread(void *junk1, unsigned long junk2)
{
	unsigned i, j;

	(void)junk1;
	(void)junk2;

	for (j=0; j<NLOOPS; j++) {
		for (i=0; i<NCVS; i++) {
			lock_acquire(testlocks[i]);
			V(gatesem);
			cv_wait(testcvs[i], testlocks[i]);
			lock_release(testlocks[i]);
		}
		kprintf("sleepthread: %u\n", j);
	}
	V(exitsem);
}

static
void
wakethread(void *junk1, unsigned long junk2)
{
	unsigned i, j;

	(void)junk1;
	(void)junk2;

	for (j=0; j<NLOOPS; j++) {
		for (i=0; i<NCVS; i++) {
			P(gatesem);
			lock_acquire(testlocks[i]);
			cv_signal(testcvs[i], testlocks[i]);
			lock_release(testlocks[i]);
		}
		kprintf("wakethread: %u\n", j);
	}
	V(exitsem);
}

int
cvtest2(int nargs, char **args)
{
	unsigned i;
	int result;

	(void)nargs;
	(void)args;

	for (i=0; i<NCVS; i++) {
		testlocks[i] = lock_create("cvtest2 lock");
		testcvs[i] = cv_create("cvtest2 cv");
	}
	gatesem = sem_create("gatesem", 0);
	exitsem = sem_create("exitsem", 0);

	kprintf("cvtest2...\n");

	result = thread_fork("cvtest2", NULL, sleepthread, NULL, 0);
	if (result) {
		panic("cvtest2: thread_fork failed\n");
	}
	result = thread_fork("cvtest2", NULL, wakethread, NULL, 0);
	if (result) {
		panic("cvtest2: thread_fork failed\n");
	}

	P(exitsem);
	P(exitsem);

	sem_destroy(exitsem);
	sem_destroy(gatesem);
	exitsem = gatesem = NULL;
	for (i=0; i<NCVS; i++) {
		lock_destroy(testlocks[i]);
		cv_destroy(testcvs[i]);
		testlocks[i] = NULL;
		testcvs[i] = NULL;
	}

	kprintf("cvtest2 done\n");
	return 0;
}

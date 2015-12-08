/*========================================
 *    sl.c: SL version 5.02
 *        Copyright 1993,1998,2014
 *                  Toyoda Masashi
 *                  (mtoyoda@acm.org)
 *        Last Modified: 2015/12/08
 *========================================
 */
/* sl version 5.02+: Apply patch of izumi@izumix.xyz                         */
/*                                                                2015/12/08 */
/* sl version 5.02 : Fix compiler warnings.                                  */
/*                                              by Jeff Schwab    2014/06/03 */
/* sl version 5.01 : removed cursor and handling of IO                       */
/*                                              by Chris Seymour  2014/01/03 */
/* sl version 5.00 : add -c option                                           */
/*                                              by Toyoda Masashi 2013/05/05 */
/* sl version 4.00 : add C51, usleep(40000)                                  */
/*                                              by Toyoda Masashi 2002/12/31 */
/* sl version 3.03 : add usleep(20000)                                       */
/*                                              by Toyoda Masashi 1998/07/22 */
/* sl version 3.02 : D51 flies! Change options.                              */
/*                                              by Toyoda Masashi 1993/01/19 */
/* sl version 3.01 : Wheel turns smoother                                    */
/*                                              by Toyoda Masashi 1992/12/25 */
/* sl version 3.00 : Add d(D51) option                                       */
/*                                              by Toyoda Masashi 1992/12/24 */
/* sl version 2.02 : Bug fixed.(dust remains in screen)                      */
/*                                              by Toyoda Masashi 1992/12/17 */
/* sl version 2.01 : Smoke run and disappear.                                */
/*                   Change '-a' to accident option.                         */
/*                                              by Toyoda Masashi 1992/12/16 */
/* sl version 2.00 : Add a(all),l(long),F(Fly!) options.                     */
/*                                              by Toyoda Masashi 1992/12/15 */
/* sl version 1.02 : Add turning wheel.                                      */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.01 : Add more complex smoke.                                 */
/*                                              by Toyoda Masashi 1992/12/14 */
/* sl version 1.00 : SL runs vomitting out smoke.                            */
/*                                              by Toyoda Masashi 1992/12/11 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include "sl.h"
#include "header.h"

#define RTOL 0
#define LTOR 1

#ifndef useconds_t
  #define USLEEP_ARG0_TYPE unsigned
#else
  #define USLEEP_ARG0_TYPE useconds_t
#endif

int add_C51(int x);
int add_D51(int x);
void option(char *str);
int my_mvaddstr(int y, int x, char *str);

int ACCIDENT  = 0;
int LOGO      = 0;
int FLY       = 0;
int C51       = 0;
int PASSNUM = 5;
int ALL_LENGTH = 0;
int DIREC = RTOL;
int WAIT_TIME = 20000;

int my_mvaddstr(int y, int x, char *str)
{
  int i = 0;
  
  for ( ; x < 0; ++x, ++i) {
    if (str[i] == '\0') {
      return ERR;
    }
  }
  if (mvaddnstr(y, x, &str[i], (int)COLS - x) == ERR) {
    return ERR;
  }
  
  return OK;
}

int my_mvaddstr_r(int y, int x, char *str)
{
  int i = 0;
  char c;
  
  for ( ; x >= COLS; --x, ++i) {
    if (str[i] == '\0') {
      return ERR;
    }
  }
  for ( ; str[i] != '\0'; ++i, --x) {
    c = str[i];
    switch (c) {
    case '\\':
      c = '/';
      break;
    case '/':
      c = '\\';
      break;
    case '(':
      c = ')';
      break;
    case ')':
      c = '(';
      break;
    case '[':
      c = ']';
      break;
    case ']':
      c = '[';
      break;
    }
    if (mvaddch(y, x, c) == ERR) {
      return ERR;
    }
  }
  return OK;
}

void option(char *str)
{
    extern int ACCIDENT, FLY, LONG;

    while (*str != '\0') {
        switch (*str++) {
            case 'a': ACCIDENT = 1; break;
            case 'F': FLY      = 1; break;
            case 'l': LOGO     = 1; break;
            case 'c': C51      = 1; break;
            default:                break;
        }
    }
}



int add_D51(int x)
{
    static char *d51[D51PATTERNS][D51HIGHT + 1]
        = {{D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL11, D51WHL12, D51WHL13, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL21, D51WHL22, D51WHL23, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL31, D51WHL32, D51WHL33, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL41, D51WHL42, D51WHL43, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL51, D51WHL52, D51WHL53, D51DEL},
           {D51STR1, D51STR2, D51STR3, D51STR4, D51STR5, D51STR6, D51STR7,
            D51WHL61, D51WHL62, D51WHL63, D51DEL}};
    static char *coal[D51HIGHT + 1]
        = {COAL01, COAL02, COAL03, COAL04, COAL05,
           COAL06, COAL07, COAL08, COAL09, COAL10, COALDEL};

    int y, i, dy = 0;

    if (x < - D51LENGTH)  return ERR;
    y = LINES / 2 - 5;

    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - D51HIGHT;
        dy = 1;
    }
    for (i = 0; i <= D51HIGHT; ++i) {
        my_mvaddstr(y + i, x, d51[(D51LENGTH + x) % D51PATTERNS][i]);
        my_mvaddstr(y + i + dy, x + 53, coal[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 2, x + 43);
        add_man(y + 2, x + 47);
    }
    add_smoke(y - 1, x + D51FUNNEL);
    return OK;
}

int add_C51(int x)
{
    static char *c51[C51PATTERNS][C51HIGHT + 1]
        = {{C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH11, C51WH12, C51WH13, C51WH14, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH21, C51WH22, C51WH23, C51WH24, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH31, C51WH32, C51WH33, C51WH34, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH41, C51WH42, C51WH43, C51WH44, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH51, C51WH52, C51WH53, C51WH54, C51DEL},
           {C51STR1, C51STR2, C51STR3, C51STR4, C51STR5, C51STR6, C51STR7,
            C51WH61, C51WH62, C51WH63, C51WH64, C51DEL}};
    static char *coal[C51HIGHT + 1]
        = {COALDEL, COAL01, COAL02, COAL03, COAL04, COAL05,
           COAL06, COAL07, COAL08, COAL09, COAL10, COALDEL};

    int y, i, dy = 0;

    if (x < - C51LENGTH)  return ERR;
    y = LINES / 2 - 5;

    if (FLY == 1) {
        y = (x / 7) + LINES - (COLS / 7) - C51HIGHT;
        dy = 1;
    }
    for (i = 0; i <= C51HIGHT; ++i) {
        my_mvaddstr(y + i, x, c51[(C51LENGTH + x) % C51PATTERNS][i]);
        my_mvaddstr(y + i + dy, x + 55, coal[i]);
    }
    if (ACCIDENT == 1) {
        add_man(y + 3, x + 45);
        add_man(y + 3, x + 49);
    }
    add_smoke(y - 1, x + C51FUNNEL);
    return OK;
}

int dirc(int y, int x)
{
  if (DIREC == RTOL) {
    my_mvaddstr(y + 5, cros0l[5] + x - 1, "<-- ");
  } else {
    my_mvaddstr(y + 5, cros0l[5] + x - 1, " -->");
  }
  
  return 0;
}

int main(int argc, char *argv[])
{
  int x, i, j, k, p, ONEDIREC, len;
  int (*sl_func)();
  time_t t;
  char num[10];
  unsigned short int s;
  char *pp;
  char *c[D51PATTERNS][D51HIGHT+1];
  
  for (i = 1; i < argc; ++i) {
  	if (*argv[i] == '-') {
      option(argv[i] + 1);
  	}
  }
  time(&t);
  s = (unsigned short int)t;
  seed48(&s);
#ifdef DEBUG
  PASSNUM = 3;
  ONEDIREC = 1;
  WAIT_TIME = (USLEEP_ARG0_TYPE)100;
  signal(SIGINT, end_proc);
#else
  signal(SIGINT, SIG_IGN);
  PASSNUM = (int)(drand48() * 20.0) + 10;
  if (drand48() > 0.5) {
    ONEDIREC = 1;
  } else {
    ONEDIREC = 0;
  }
  WAIT_TIME = (USLEEP_ARG0_TYPE)(drand48() * 50000.0);
#endif
  ALL_LENGTH = (3 * D51LENGTH + (PASSLENGTH * (PASSNUM - 1)) + LPASSLENGTH);
  initscr();
  noecho();
#ifdef DEBUG
  printf("%d,%d\n\r", COLS, LINES);
  printf("Hit any key\n\r");
  fflush(stdout);
  getc(stdin);
#endif
  leaveok(stdscr, TRUE);
  scrollok(stdscr, FALSE);
  DIREC = RTOL;
  p = 3 * COLS / 10;
  pp = (char*)malloc((size_t)(COLS + ALL_LENGTH + 10) * (D51HIGHT + 1) * (D51PATTERNS + 1));
  for (i = 0; i <= (COLS + ALL_LENGTH + 1) * (D51HIGHT + 1) * D51PATTERNS; ++i) {
    pp[i] = (char)NULL;
  }
  
  for (j = 0; j < D51PATTERNS; ++j) {
    for (i = 0; i <= D51HIGHT; ++i) {
      c[j][i] = pp + (COLS + ALL_LENGTH + 2) * i + (COLS + ALL_LENGTH + 1) * (D51HIGHT + 1) * j;
      for (k = 0; k < COLS; ++k) {
        strcat(c[j][i], " ");
      }
      strncat(c[j][i], d51[j][i], 53);
      strncat(c[j][i], coal[i], 29);
      strncat(c[j][i], d51[j][i], 53);
      strncat(c[j][i], coal[i], 29);
      strncat(c[j][i], d51[j][i], 53);
      strncat(c[j][i], coal[i], 29);
      for (k = 0; k < PASSNUM - 1; ++k) {
        strncat(c[j][i], coach[i], 88);
        if ( i == 3 ) {
          sprintf(num, "%d", k + 1);
          len = strlen(num);
          strncpy(c[j][i] + COLS + 254 + (PASSLENGTH * k), num, len);
        }
      }
      strncat(c[j][i], lcoach[i], 89);
      if ( i == 3 ) {
        sprintf(num, "%d", k + 1);
        len = strlen(num);
        strncpy(c[j][i] + COLS + 254 + (PASSLENGTH * k), num, len);
      }
    }
  }
  if (FLY != 1) {
    begin_gate(p);
  }
  if (LOGO == 0) {
    sl_func = add_D51_coach;
  } else {
    sl_func = add_sl;
  }
  for (x = COLS - 1 ; ; --x) {
    if ((*sl_func)(x, c) == ERR) break;
    if (FLY != 1) {
      if (add_cross(p) == ERR) break;
    }
    refresh();
    usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
  }
  if (FLY != 1 && LOGO == 0 && ONEDIREC == 1) {
    x_gate(p);
    for (x = 0; ; ++x) {
      if (add_D51_coach_r(x) == ERR) break;
      if (add_cross(p) == ERR) break;
      refresh();
      usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
    }
  }
  if (FLY != 1) {
    end_gate(p);
  }
  mvcur(0, COLS - 1, LINES - 1, 0);
  endwin();
  
  return 0;
}

 
void end_proc() {
  mvcur(0, COLS - 1, LINES - 1, 0);
  endwin();
  exit(SIGINT);
}
 int add_cross(int p)
 {
   int i, y, n = 20, hn;
   static int tt;
 #ifdef DEBUG
   char buf[100];
 #endif
 
   hn = n / 2;
   if ( LOGO == 0) {
     y = LINES / 2 - 5;
   } else {
     y = LINES / 2 - 7;
   }
   for (i = 2; i < D51HIGHT; ++i) {
     my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
   }
   for (i = 8; i < D51HIGHT; ++i) {
     my_mvaddstr(y + i, p + 5, cros3[i]);
   }
 #ifdef DEBUG
   sprintf(buf, "%d", tt);
   my_mvaddstr(0, 0, buf);
 #endif
   if ( tt % n >= 0 && tt % n < hn) {
     my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
     my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
     my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
   } else if ( tt % n >= hn && tt % n < n) {
     my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
     my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
     dirc(y, p);
   }
   ++tt;
 
   return 0;
 }
 
 
 int begin_gate(int p)
 {
   int i, y, n = 20, hn;
   int tt;
 
   hn = n / 2;
   if ( LOGO == 0) {
     y = LINES / 2 - 5;
   } else {
     y = LINES / 2 - 7;
   }
   
   for (tt = 0; tt < 80; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros1[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
   for (tt = 0; tt <= 15; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros2[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
   for (tt = 0; tt <= 20; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros3[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
 
   return 0;
 }
 
 
 int end_gate(int p)
 {
   int i, y, n = 20, hn;
   int tt;
 
   hn = n / 2;
   if ( LOGO == 0) {
     y = LINES / 2 - 5;
   } else {
     y = LINES / 2 - 7;
   }
   
   for (tt = 0; tt <= 20; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros3[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
   for (tt = 0; tt <= 15; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros2[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
   for (tt = 0; tt < 80; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros1[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
     my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
     my_mvaddstr(LINES + 1, COLS + 1, "");
     my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
  
   return 0;
 }
 
 
 int x_gate(int p)
 {
   int i, y, n = 20, hn;
   int tt;
 
   hn = n / 2;
   if ( LOGO == 0) {
     y = LINES / 2 - 5;
   } else {
     y = LINES / 2 - 7;
   }
   
   for (tt = 0; tt <= 20; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros3[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
   for (tt = 0; tt <= 10; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros2[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
   DIREC = ( DIREC + 1 ) % 2;
   for (tt = 0; tt <= 10; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros2[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
   for (tt = 0; tt <= 20; ++tt) {
     for (i = 0; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, p + 5, cros3[i]);
     }
     for (i = 2; i < D51HIGHT; ++i) {
       my_mvaddstr(y + i, cros0l[i] + p, cros0[i]);
     }
     if ( tt % n >= 0 && tt % n < hn) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "O");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "X");
       my_mvaddstr(y + 5, cros0l[5] + p - 1, " || ");
     } else if ( tt % n >= hn && tt % n < n) {
       my_mvaddstr(y + 4, cros0l[5] + p - 1, "X");
       my_mvaddstr(y + 4, cros0l[5] + p + 2, "O");
       dirc(y, p);
     }
     my_mvaddstr(LINES + 1, COLS + 1, "");
     refresh();
     usleep((USLEEP_ARG0_TYPE)WAIT_TIME);
   }
 
   return 0;
 }
 
 
 int add_sl(int x, char *c[])
 {
      int i, y, py1 = 0, py2 = 0, py3 = 0;
      
     if (x < - LOGOLENGTH) {
       return ERR;
     }
      y = LINES / 2 - 3;
  
      if (FLY == 1) {
         y = (x / 6) + LINES - (COLS / 6) - LOGOHIGHT;
         py1 = 2;  py2 = 4;  py3 = 6;
      }
      for (i = 0; i <= LOGOHIGHT; ++i) {
         my_mvaddstr(y + i, x, sl[(LOGOLENGTH + x) / 3 % LOGOPATTERNS][i]);
         my_mvaddstr(y + i + py1, x + 21, lcoal[i]);
         my_mvaddstr(y + i + py2, x + 42, car[i]);
         my_mvaddstr(y + i + py3, x + 63, car[i]);
      }
      if (ACCIDENT == 1) {
         add_man(y + 1, x + 14);
         add_man(y + 1 + py2, x + 45);
 	add_man(y + 1 + py2, x + 53);
         add_man(y + 1 + py3, x + 66);
 	add_man(y + 1 + py3, x + 74);
      }
      add_smoke(y - 1, x + LOGOFUNNEL);
      return OK;
 }
  
  
 int add_D51_coach(int x, char *c[]) {
      int y, i, dy = 0;
 #ifdef DEBUG
     char buf[100];
 #endif
 
     if (x < ( - ALL_LENGTH + 4 ))  return ERR;
     y = LINES / 2 - 5;
  
 #ifdef DEBUG
     sprintf(buf, "%d", x);
     my_mvaddstr(1, 0, buf);
 #endif
     if (FLY == 1) {
  	y = (x / 7) + LINES - (COLS / 7) - D51HIGHT;
  	dy = 1;
      }
 
      for (i = 0; i <= D51HIGHT; ++i) {
       my_mvaddstr(y + i, 0, c[(D51HIGHT + 1) * ((ALL_LENGTH + x) % D51PATTERNS) + i] + COLS - x);
     }
      if (ACCIDENT == 1) {
 	add_man(y + 2, x + 43);
  	add_man(y + 2, x + 47);
 	add_man(y + 2, x + 125);
 	add_man(y + 2, x + 129);
 	add_man(y + 2, x + 207);
 	add_man(y + 2, x + 211);
      }
      add_smoke(y - 1, x + D51FUNNEL);
     add_smoke(y - 1, x + D51FUNNEL + 81);
     add_smoke(y - 1, x + D51FUNNEL + 162);
 
     return OK;
 }
 
 
 int add_D51_coach_r(int x)
 {
     int y, i, j;
     char num[10];
 
     if (x > ALL_LENGTH + COLS)  return ERR;
     y = LINES / 2 - 5;
 
     for (i = 0; i <= D51HIGHT; ++i) {
 	my_mvaddstr_r(y + i, x, d51_r[(ALL_LENGTH + x) % D51PATTERNS][i]);
 	my_mvaddstr_r(y + i, x - 53, coal[i]);
 	my_mvaddstr_r(y + i, x - 82, d51_r[(ALL_LENGTH + x) % D51PATTERNS][i]);
 	my_mvaddstr_r(y + i, x - 135, coal[i]);
 	my_mvaddstr_r(y + i, x - 164, d51_r[(ALL_LENGTH + x) % D51PATTERNS][i]);
 	my_mvaddstr_r(y + i, x - 217, coal[i]);
         for (j = 0; j < PASSNUM - 1; ++j) {
 	  my_mvaddstr_r(y + i, x - 246 - (PASSLENGTH * j), coach[i]);
           if ( i == 3 ) {
 	    sprintf(num, "%d", j + 1);
 	    my_mvaddstr(y + i, x - 255 - (PASSLENGTH * j), num);
 	  }
         }
 	my_mvaddstr_r(y + i, x - 246 - (PASSLENGTH * (PASSNUM - 1)), lcoach[i]);
 	if ( i == 3 ) {
 	  sprintf(num, "%d", j + 1);
 	  my_mvaddstr(y + i, x - 255 - (PASSLENGTH * (PASSNUM - 1)), num);
 	}
     }
     if (ACCIDENT == 1) {
 	add_man(y + 2, x - 45);
 	add_man(y + 2, x - 49);
 	add_man(y + 2, x - 127);
 	add_man(y + 2, x - 131);
 	add_man(y + 2, x - 209);
 	add_man(y + 2, x - 213);
     }
     add_smoke_r(y - 1, x - D51FUNNEL - 3);
     add_smoke_r(y - 1, x - D51FUNNEL - 84);
     add_smoke_r(y - 1, x - D51FUNNEL - 167);
     return OK;
  }
  

int add_man(int y, int x)
{
    static char *man[2][2] = {{"", "(O)"}, {"Help!", "\\O/"}};
    int i;

    if ( x < 0 ) {
      return 0;
    }
    for (i = 0; i < 2; ++i) {
        my_mvaddstr(y + i, x, man[(LOGOLENGTH + x) / 12 % 2][i]);
    }
    return 0;
}


int add_smoke(int y, int x)
#define SMOKEPTNS        16
{
    static struct smokes {
        int y, x;
        int ptrn, kind;
    } S[1000];
    static int sum = 0;
    static char *Smoke[2][SMOKEPTNS]
        = {{"(   )", "(    )", "(    )", "(   )", "(  )",
            "(  )" , "( )"   , "( )"   , "()"   , "()"  ,
            "O"    , "O"     , "O"     , "O"    , "O"   ,
            " "                                          },
           {"(@@@)", "(@@@@)", "(@@@@)", "(@@@)", "(@@)",
            "(@@)" , "(@)"   , "(@)"   , "@@"   , "@@"  ,
            "@"    , "@"     , "@"     , "@"    , "@"   ,
            " "                                          }};
    static char *Eraser[SMOKEPTNS]
        =  {"     ", "      ", "      ", "     ", "    ",
            "    " , "   "   , "   "   , "  "   , "  "  ,
            " "    , " "     , " "     , " "    , " "   ,
            " "                                          };
    static int dy[SMOKEPTNS] = { 2,  1, 1, 1, 0, 0, 0, 0, 0, 0,
                                 0,  0, 0, 0, 0, 0             };
    static int dx[SMOKEPTNS] = {-2, -1, 0, 1, 1, 1, 1, 1, 2, 2,
                                 2,  2, 2, 3, 3, 3             };
    int i;
     if (x < - COLS) {
       return 0;
     }

    if (x % 4 == 0) {
        for (i = 0; i < sum; ++i) {
            my_mvaddstr(S[i].y, S[i].x, Eraser[S[i].ptrn]);
            S[i].y    -= dy[S[i].ptrn];
            S[i].x    += dx[S[i].ptrn];
            S[i].ptrn += (S[i].ptrn < SMOKEPTNS - 1) ? 1 : 0;
            my_mvaddstr(S[i].y, S[i].x, Smoke[S[i].kind][S[i].ptrn]);
        }
        my_mvaddstr(y, x, Smoke[sum % 2][0]);
        S[sum].y = y;    S[sum].x = x;
        S[sum].ptrn = 0; S[sum].kind = sum % 2;
        sum ++;
    }
    return 0;
    
}

 int add_smoke_r(int y, int x)
 #define SMOKEPTNS	16
 {
     static struct smokes {
 	int y, x;
 	int ptrn, kind;
     } S[1000];
     static int sum = 0;
     static char *Smoke[2][SMOKEPTNS]
 	= {{"(   )", "(    )", "(    )", "(   )", "(  )",
 	    "(  )" , "( )"   , "( )"   , "()"   , "()"  ,
 	    "O"    , "O"     , "O"     , "O"    , "O"   ,
 	    " "                                          },
 	   {"(@@@)", "(@@@@)", "(@@@@)", "(@@@)", "(@@)",
 	    "(@@)" , "(@)"   , "(@)"   , "@@"   , "@@"  ,
 	    "@"    , "@"     , "@"     , "@"    , "@"   ,
 	    " "                                          }};
     static char *Eraser[SMOKEPTNS]
 	=  {"     ", "      ", "      ", "     ", "    ",
 	    "    " , "   "   , "   "   , "  "   , "  "  ,
 	    " "    , " "     , " "     , " "    , " "   ,
 	    " "                                          };
     static int dy[SMOKEPTNS] = { 2,  1, 1, 1, 0, 0, 0, 0, 0, 0,
 				 0,  0, 0, 0, 0, 0             };
     static int dx[SMOKEPTNS] = {-2, -1, 0, 1, 1, 1, 1, 1, 2, 2,
 				 2,  2, 2, 3, 3, 3             };
     int i;
 
     if (x > 2 * COLS) {
       return 0;
     }
     if (x % 4 == 0) {
 	for (i = 0; i < sum; ++i) {
 	    my_mvaddstr_r(S[i].y, S[i].x, Eraser[S[i].ptrn]);
 	    S[i].y    -= dy[S[i].ptrn];
 	    S[i].x    -= dx[S[i].ptrn];
 	    S[i].ptrn += (S[i].ptrn < SMOKEPTNS - 1) ? 1 : 0;
 	    my_mvaddstr_r(S[i].y, S[i].x, Smoke[S[i].kind][S[i].ptrn]);
 	}
 	my_mvaddstr(y, x, Smoke[sum % 2][0]);
 	S[sum].y = y;    S[sum].x = x;
 	S[sum].ptrn = 0; S[sum].kind = sum % 2;
 	sum ++;
     }
     return 0;
 }

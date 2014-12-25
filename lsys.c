/* Change Log.
    July 18, 2007 (Farhan Ahammed)
      - No longer a stand-alone program. Can be #included into other files.
        - Commented out main() method.
        - The method Lsystem() must now be called.
      - Output now writes to a file.
        - Filename must be specified when calling Lsystem().
        - Created 'outputfile' variable to store FILE pointer reference.
      - Writes TSPlib formatted data.
        - Instead of just printing coordinates, it now prints:
             <node ID> <x-coord> <y-coord>
        - Created 'currentnode' variable to keep track of node ID.

    January 7, 2008 (Farhan Ahammed)
      - Implemented the "Perturbation" functionality
        - Updated the L-System grammar to allow use of the new functionality.
          - Defined an epsilon which is specified in the L-System (similar to defining the angle).
          - Defined the letter 'P' to specify that the next point should be moved (perturbated).
        - Implemented method 'lsys_doperturbation' which "moves" the point to another place
          according to the epsilon and a random angle.
        - Included code so that the Mersenne Twister pseudorandom number generator is used to
          create the random angles.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __TURBOC__
#include <alloc.h>
#define nint floor
#elif !defined(__386BSD__)
#include <malloc.h>
#endif

#define PI 3.14159265358979323846
#define nint floor


#define xdots 1000
#define ydots 1000

/* The points will be sent to a file (instead of stdout). */
FILE *outputfile;

/* For each node being created, their label will also be assigned,
   This way a valid TSP file can be created.
   i.e. <currentnode> <x-coord> <y-coord>                        */
static int currentnode;


char *strlwr(char *s)
{
  for(;*s;s++) *s=tolower(*s);
  return s;
}


struct lsys_cmd {
  char ch;
  void (*f)(double n);
  double n;
};


void draw_line(double a,double b,double c,double d)
{
  static int lessthanzero=0;
  /*printf("%d %d\n",(int)nint(c),(int)nint(d));*/
  fprintf(outputfile, "%lu %d %d\n", currentnode, (int)nint(c),(int)nint(d));
  currentnode++;

  if (!lessthanzero && (nint(c)<0 || nint(d)<0)) {
     fprintf(stderr,"Warning:  output contains negative numbers\n");
     lessthanzero=1;
  }
}


static double	   getnumber(char  **);
static int	   findscale(struct lsys_cmd  *, struct lsys_cmd  **, int);
static struct lsys_cmd  *  drawLSys(struct lsys_cmd  *, struct lsys_cmd  **, int);
static int	   readLSystemFile(char *, char *);
static void	   free_rules_mem(void);
static int	   save_rule(char *,char  **);
static struct lsys_cmd  *SizeTransform(char  *s);
static struct lsys_cmd  *DrawTransform(char  *s);
static void free_lcmds();
static void stopmsg(char *);

double sins[50];
double coss[50];

/* dmaxangle is maxangle - 1. */
char maxangle,dmaxangle;
double size;
double realangle;
double xpos,ypos;
char counter,angle,reverse,stackoflow;
double lsys_Xmin, lsys_Xmax, lsys_Ymin, lsys_Ymax;



static void lsys_doplus(double n)
{
  if (reverse) {
    if (++angle == maxangle)
      angle = 0;
  }
  else {
    if (angle)
      angle--;
    else
      angle = dmaxangle;
  }
}


static void lsys_dominus(double n)
{
  if (reverse) {
    if (angle)
      angle--;
    else
      angle = dmaxangle;
  }
  else {
    if (++angle == maxangle)
      angle = 0;
  }
}


static void lsys_doslash(double n)
{
  if (reverse)
    realangle -= n;
  else
    realangle += n;
}


static void lsys_dobslash(double n)
{
  if (reverse)
    realangle += n;
  else
    realangle -= n;
}


static void lsys_doat(double n)
{
  size *= n;
}


static void lsys_dopipe(double n)
{
  angle += maxangle / 2;
  angle %= maxangle;
}


static void lsys_dobang(double n)
{
  reverse = ! reverse;
}


static void lsys_dosizedm(double n)
{
  xpos+=size*cos(realangle*PI/180);
  ypos+=size*sin(realangle*PI/180);
  if (xpos>lsys_Xmax) lsys_Xmax=xpos;
  if (ypos>lsys_Ymax) lsys_Ymax=ypos;
  if (xpos<lsys_Xmin) lsys_Xmin=xpos;
  if (ypos<lsys_Ymin) lsys_Ymin=ypos;
}

static void lsys_dosizegf(double n)
{
  xpos+=size*coss[angle];
  ypos+=size*sins[angle];
  if (xpos>lsys_Xmax) lsys_Xmax=xpos;
  if (ypos>lsys_Ymax) lsys_Ymax=ypos;
  if (xpos<lsys_Xmin) lsys_Xmin=xpos;
  if (ypos<lsys_Ymin) lsys_Ymin=ypos;
}


static void lsys_dodrawd(double n)
{
  double lastx=xpos;
  double lasty=ypos;

  xpos+=size*cos(realangle*PI/180);
  ypos+=size*sin(realangle*PI/180);
  draw_line(lastx,lasty,xpos,ypos);
#ifdef STDERROUT
fprintf(stderr,"%.18g %.18g\n",xpos,ypos);
#endif

}

static void lsys_dodrawm(double n)
{
  xpos+=size*cos(realangle*PI/180);
  ypos+=size*sin(realangle*PI/180);
}

static void lsys_dodrawg(double n)
{
  xpos+=size*coss[angle];
  ypos+=size*sins[angle];
}


static void lsys_dodrawf(double n)
{
  double lastx=xpos;
  double lasty=ypos;

  xpos+=size*coss[angle];
  ypos+=size*sins[angle];
  draw_line(lastx,lasty,xpos,ypos);
#ifdef STDERROUT
fprintf(stderr,"%.18g %.18g\n",xpos,ypos);
#endif
}



static double  getnumber(char  **str)
{
   char numstr[30];
   float ret;
   int i,root,inverse;

   root=0;
   inverse=0;
   strcpy(numstr,"");
   (*str)++;
   switch (**str)
   {
   case 'q':
      root=1;
      (*str)++;
      break;
   case 'i':
      inverse=1;
      (*str)++;
      break;
   }
   switch (**str)
   {
   case 'q':
      root=1;
      (*str)++;
      break;
   case 'i':
      inverse=1;
      (*str)++;
      break;
   }
   i=0;
   while (**str<='9' && **str>='0' || **str=='.')
   {
      numstr[i++]= **str;
      (*str)++;
   }
   (*str)--;
   numstr[i]=0;
   ret=atof(numstr);
   if (root)
     ret=sqrt(ret);
   if (inverse)
     ret = 1/ret;
   return ret;
}


static int  findscale(struct lsys_cmd  *command, struct lsys_cmd  **rules, int depth)
{
   float horiz,vert;
   double xmin, xmax, ymin, ymax;
   double locsize;
   int i;
   struct lsys_cmd  *fsret;
   for(i=0;i<maxangle;i++) {
      sins[i]=sin(2*i*PI/maxangle);
      coss[i]=cos(2*i*PI/maxangle);
   }
   xpos=ypos=lsys_Xmin=lsys_Xmax=lsys_Ymax=lsys_Ymin=angle=reverse=realangle=counter=0;
   size=1.0;
   fsret = drawLSys(command,rules,depth);
   xmin = lsys_Xmin;
   xmax = lsys_Xmax;
   ymin = lsys_Ymin;
   ymax = lsys_Ymax;
   locsize = size;
   if (fsret == NULL)
      return 0;
   if (xmax == xmin)
      horiz = 1E37;
   else
      horiz = (xdots-10)/(xmax-xmin);
   if (ymax == ymin)
      vert = 1E37;
   else
      vert = (ydots-6) /(ymax-ymin);
   locsize = (vert<horiz) ? vert : horiz;

   if (horiz == 1E37)
      xpos = ceil(xdots/2);
   else
      xpos = ceil(-xmin*(locsize)+5+((xdots-10)-(locsize)*(xmax-xmin))/2);
   if (vert == 1E37)
      ypos = ceil(ydots/2);
   else
      ypos = ceil(-ymin*(locsize)+3+((ydots-6)-(locsize)*(ymax-ymin))/2);
   locsize=floor(locsize);
   if (locsize==0) {
     locsize=1;
     fprintf(stderr,"Size too small\n");
   }
   size = locsize;
#ifdef STDERROUT
  fprintf(stderr,"start: %.18g,%.18g   %.18g\n",xpos,ypos,size);
#endif
   return 1;
}

static struct lsys_cmd  *  drawLSys(struct lsys_cmd  *command,struct lsys_cmd  **rules,int depth)
{
   struct lsys_cmd  **rulind;
   int tran;

   while (command->ch && command->ch !=']') {
      tran=0;
      if (depth) {
	 for(rulind=rules;*rulind;rulind++)
	    if ((*rulind)->ch == command->ch) {
	       tran=1;
	       if (drawLSys((*rulind)+1,rules,depth-1) == NULL)
		  return NULL;
	    }
      }
      if (!depth||!tran) {
	if (command->f)
	  (*command->f)(command->n);
	else if (command->ch == '[') {
	  char saveang,saverev;
	  double savesize,savex,savey;
	  double saverang;
	  saveang=angle;
	  saverev=reverse;
	  savesize=size;
	  saverang=realangle;
	  savex=xpos;
	  savey=ypos;
	  if ((command=drawLSys(command+1,rules,depth)) == NULL)
	     return(NULL);
	  angle=saveang;
	  reverse=saverev;
	  size=savesize;
	  realangle=saverang;
	  xpos=savex;
	  ypos=savey;
	}
      }
      command++;
   }
   return command;
}

#define MAXRULES 27 /* this limits rules to 25 */
static char  *ruleptrs[MAXRULES];
static struct lsys_cmd  *rules2[MAXRULES];

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

static int  readLSystemFile(char *str,char *name)
{
   int c;
   char  **rulind;
   int err=0;
   int linenum,check=0;
   char theline[161],fixed[161],*word;
   FILE *infile;
   char msgbuf[481]; /* enough for 6 full lines */

   infile = fopen(str,"rt");
   if (!infile) {
     fprintf(stderr, "Unable to open file '%s'\n\n",str);
     return -1;
   }
   if (name) {
     while (fgets(msgbuf,sizeof(msgbuf),infile)){
       if (strstr(msgbuf,name)){
         fseek(infile,ftell(infile)-strlen(msgbuf),SEEK_SET);
         name=0;
         break;
       }
     }
     if (name) {
       fprintf(stderr, "Unable to find lsystem '%s' in file '%s'\n\n",
              name,str);
       return -1;
     }
   }
   while ((c = fgetc(infile)) != '{')
      if (c == EOF) return -1;

   maxangle=0;
   for(linenum=0;linenum<MAXRULES;++linenum) ruleptrs[linenum]=NULL;
   rulind= &ruleptrs[1];
   msgbuf[0]=linenum=0;

   while(fgets(theline,160,infile))  /* Max line length 160 chars */
   {
      linenum++;
      if ((word = strchr(theline,';'))) /* strip comment */
	 *word = 0;
      strlwr(theline);

      if (strspn(theline," \t\n") < strlen(theline)) /* not a blank line */
      {
	 word=strtok(theline," =\t\n");
	 if (!strcmp(word,"axiom"))
	 {
	    save_rule(strtok(NULL," \t\n"),&ruleptrs[0]);
	    check=1;
	 }
	 else if (!strcmp(word,"angle"))
	 {
	    maxangle=atoi(strtok(NULL," \t\n"));
	    dmaxangle = maxangle - 1;
	    check=1;
	 }
	 else if (!strcmp(word,"}"))
	    break;
	 else if (strlen(word)==1)
	 {
	    char *tok;
	    tok = strtok(NULL, " \t\n");
	    strcpy(fixed, word);
	    if (tok != NULL)
               strcat(fixed, tok);
	    save_rule(fixed, rulind++);
	    check=1;
	 }
	 else
	    if (err<6)
	    {
	       sprintf(&msgbuf[strlen(msgbuf)],
		       "Syntax error line %d: %s\n",linenum,word);
	       ++err;
	    }
	 if (check)
	 {
	    check=0;
	    if(word=strtok(NULL," \t\n"))
	       if (err<6)
	       {
		  sprintf(&msgbuf[strlen(msgbuf)],
			 "Extra text after command line %d: %s\n",linenum,word);
		  ++err;
	       }
	 }
      }
   }
   fclose(infile);
   if (!ruleptrs[0] && err<6)
   {
      strcat(msgbuf,"Error:  no axiom\n");
      ++err;
   }
   if ((maxangle<3||maxangle>50) && err<6)
   {
      strcat(msgbuf,"Error:  illegal or missing angle\n");
      ++err;
   }
   if (err)
   {
      msgbuf[strlen(msgbuf)-1]=0; /* strip trailing \n */
      stopmsg(msgbuf);
      return -1;
   }
   *rulind=NULL;
   return 0;
}


int Lsystem(int order, char *name, char *title, char *outputfilename)
{
   currentnode = 0;
   outputfile = fopen(outputfilename, "w");
   if (outputfile == NULL)
      fprintf(stderr, "ERROR. Couldn't open outputfile (%s)\n", outputfilename);

   char  **rulesc;
   struct lsys_cmd  **sc;

   if (LLoad(name,title))
     return -1;

   stackoflow = 0;

   sc = rules2;
   for (rulesc = ruleptrs; *rulesc; rulesc++)
     *sc++ = SizeTransform(*rulesc);
   *sc = NULL;

   if (findscale(rules2[0], &rules2[1], order)) {
      realangle = angle = reverse = 0;

      free_lcmds();
      sc = rules2;
      for (rulesc = ruleptrs; *rulesc; rulesc++)
        *sc++ = DrawTransform(*rulesc);
      *sc = NULL;

      drawLSys(rules2[0], &rules2[1], order);
   }
   if (stackoflow)
   {
      static char  msg[]={"insufficient memory, try a lower order"};
      stopmsg(msg);
   }
   free_rules_mem();
   free_lcmds();


   if (outputfile != NULL)
      if (fclose(outputfile) != 0)
         fprintf(stderr, "ERROR. Couldn't close outputfile (%s)\n", outputfilename);

   return currentnode;
}

int LLoad(char *name,char *title)
{
   char i;
   if (readLSystemFile(name,title)) { /* error occurred */
      free_rules_mem();
      return -1;
   }
   for(i=0;i<maxangle;i++) {
      sins[i]=sin(2*i*PI/maxangle);
      coss[i]=cos(2*i*PI/maxangle);
   }
   return 0;
}

static void  free_rules_mem()
{
   int i;
   for(i=0;i<MAXRULES;++i)
      if(ruleptrs[i]) free(ruleptrs[i]);
}

static int save_rule(char *rule,char  **saveptr)
{
   int i;
   char  *tmp;
   i=strlen(rule)+1;
   if((tmp=malloc((long)i))==NULL) {
       stackoflow = 1;
       return -1;
       }
   *saveptr=tmp;
   while(--i>=0) *(tmp++)= *(rule++);
   return 0;
}

static struct lsys_cmd  *SizeTransform(char  *s)
{
  struct lsys_cmd  *ret;
  struct lsys_cmd  *doub;
  int maxval = 10;
  int n = 0;
  void (*f)(double);
  double num;

  void (*plus)(double) = lsys_doplus;
  void (*minus)(double) = lsys_dominus;
  void (*pipe)(double) = lsys_dopipe;

  void (*slash)(double) =   lsys_doslash;
  void (*bslash)(double) = lsys_dobslash;
  void (*at)(double) =      lsys_doat;
  void (*dogf)(double) =    lsys_dosizegf;

  ret = (struct lsys_cmd  *) malloc((long) maxval * sizeof(struct lsys_cmd));
  if (ret == NULL) {
       stackoflow = 1;
       return NULL;
       }
  while (*s) {
    f = NULL;
    num = 0;
    ret[n].ch = *s;
    switch (*s) {
      case '+': f = plus;            break;
      case '-': f = minus;           break;
      case '/': f = slash;           num = getnumber(&s);break;
      case '\\': f = bslash;         num = getnumber(&s);    break;
      case '@': f = at;              num = getnumber(&s);    break;
      case '|': f = pipe;            break;
      case '!': f = lsys_dobang;     break;
      case 'd':
      case 'm': f = lsys_dosizedm;   break;
      case 'g':
      case 'f': f = dogf;       break;
      case '[': num = 1;        break;
      case ']': num = 2;        break;
      default:
	num = 3;
	break;
    }
    ret[n].f = f;
    ret[n].n = num;
    if (++n == maxval) {
      doub = (struct lsys_cmd  *) malloc((long) maxval*2*sizeof(struct lsys_cmd));
      if (doub == NULL) {
         free(ret);
         stackoflow = 1;
         return NULL;
         }
      memcpy(doub, ret, maxval*sizeof(struct lsys_cmd));
      free(ret);
      ret = doub;
      maxval <<= 1;
    }
    s++;
  }
  ret[n].ch = 0;
  ret[n].f = NULL;
  ret[n].n = 0;
  n++;

  doub = (struct lsys_cmd  *) malloc((long) n*sizeof(struct lsys_cmd));
  if (doub == NULL) {
       free(ret);
       stackoflow = 1;
       return NULL;
       }
  memcpy(doub, ret, n*sizeof(struct lsys_cmd));
  free(ret);
  return doub;
}

static struct lsys_cmd  *DrawTransform(char  *s)
{
  struct lsys_cmd  *ret;
  struct lsys_cmd  *doub;
  int maxval = 10;
  int n = 0;
  void (*f)(double);
  double num;

  void (*plus)(double) = lsys_doplus;
  void (*minus)(double) = lsys_dominus;
  void (*pipe)(double) = lsys_dopipe;

  void (*slash)(double) =   lsys_doslash;
  void (*bslash)(double) =  lsys_dobslash;
  void (*at)(double) =      lsys_doat;
  void (*drawg)(double) =   lsys_dodrawg;

  ret = (struct lsys_cmd  *) malloc((long) maxval * sizeof(struct lsys_cmd));
  if (ret == NULL) {
       stackoflow = 1;
       return NULL;
       }
  while (*s) {
    f = NULL;
    num = 0;
    ret[n].ch = *s;
    switch (*s) {
      case '+': f = plus;                break;
      case '-': f = minus;               break;
      case '/': f = slash;               num = getnumber(&s);    break;
      case '\\': f = bslash;             num = getnumber(&s);   break;
      case '@': f = at;                  num = getnumber(&s);    break;
      case '|': f = pipe;                break;
      case '!': f = lsys_dobang;         break;
      case 'd': f = lsys_dodrawd;        break;
      case 'm': f = lsys_dodrawm;        break;
      case 'g': f = drawg;               break;
      case 'f': f = lsys_dodrawf;        break;
      case '[': num = 1;        break;
      case ']': num = 2;        break;
      default:
	num = 3;
	break;
    }
    ret[n].f = f;
    ret[n].n = num;
    if (++n == maxval) {
      doub = (struct lsys_cmd  *) malloc((long) maxval*2*sizeof(struct lsys_cmd));
      if (doub == NULL) {
           free(ret);
           stackoflow = 1;
           return NULL;
           }
      memcpy(doub, ret, maxval*sizeof(struct lsys_cmd));
      free(ret);
      ret = doub;
      maxval <<= 1;
    }
    s++;
  }
  ret[n].ch = 0;
  ret[n].f = NULL;
  ret[n].n = 0;
  n++;

  doub = (struct lsys_cmd  *) malloc((long) n*sizeof(struct lsys_cmd));
  if (doub == NULL) {
       free(ret);
       stackoflow = 1;
       return NULL;
       }
  memcpy(doub, ret, n*sizeof(struct lsys_cmd));
  free(ret);
  return doub;
}

static void free_lcmds()
{
  struct lsys_cmd  **sc = rules2;

  while (*sc)
    free(*sc++);
}


void stopmsg(char *msg)
{
  fprintf(stderr,msg);
  exit(1);
}

/*
main(int argc, char **argv)
{
  if (argc<3) {
    fprintf(stderr,"\nlsys 1.0\n\nUsage: lsys order filename [lsystem-name]\n");
    fprintf(stderr,"\n\torder - order of lsystem to generate");
    fprintf(stderr,"\n\tfilename - file containing lsystem definition");
    fprintf(stderr,"\n\tlsystem-name - name of specific lsystem to generate");
    fprintf(stderr,"\n\n");
    exit(3);
  }
  Lsystem(atoi(argv[1]),argv[2],argv[3]);
}
*/




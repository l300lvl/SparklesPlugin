/*
 * Sparkles for XChat
 *
 * Copyright (C) 2011-2015 Princess Nova Storm the Squirrel
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//   YiffScript originally by Dr Dos
//   Extra yiff actions taken from http://pastie.org/pastes/1069466

#define PNAME "Nova's Sparkles"
#define PDESC "Mix of annoying/powerful/useful stuff"
#define PVERSION "1.1"
#define USE_SPARKLES_USER 0
#define ENABLE_NSFW_CONTENT 1
#define PESTERCHUM_NETWORK "Pesterchum"

#include "xchat-plugin.h"
#include <stdio.h>         // sprintf() and file access
#include <string.h>        // compares and copies and stuff
#include <stdarg.h>        // Adj() really needs this
#include <stdlib.h>        // for strtol() and rand() mainly
#include <time.h>          // so I can seed the randomizer with the current time
#include <ctype.h>         // used in the capitalization functions
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#endif
static xchat_plugin *ph;   /* plugin handle */
static char TextEditor[512]="start notepad";
static int RejoinKick =  0;
static int EatInvites =  0;
static int JoinOnInvite= 0;
static int BeVerbose =   0;
static int LastYiff  =  -1;
static int DisableAutoIdent = 0;
static int DisableAutoNickDeblue= 0;
static int DisableAutoGhost=0;
static int DisableAutoGhostChainBreak=0;
static int DisablePesterchum=0;
static int DisablePrettyJanus=0;
static int DisableSparklesCTCP = 0;
static int ForceUTF8 = 0;
static int AutoReclaimNick = 0;
static char GhostReclaimNick[80]="";
static int DisablePlusJFix = 0;
static int Activity2Focus = 0;
static char SayHookCommand[512]="";
static int SayHookSpace = 1;
static char OneSayHook[512]="";
static char MeHookCommand[512]="";
static int UseOneSayHook = 0;
static xchat_hook *SayHook, *MeHook;
static int EatHighlights = 0;
//static int MoveNotifyToServer=0;
//static int MoveServicesToServer=0;
static char ConfigFilePath[512]="";
static char CustomYiffName[32] = "";
static char *CustomYiffBuffer = NULL;
static int CustomYiffCount = 0;
static char *CustomYiffPointers[500];

static unsigned int GhostDelayTime = 0; // to help stop "oh my god I started xchat and it autoghosted stuff"
static int GrabbingTopic = 0;
static char CommandPrefix[5] = "/"; // because this is able to be changed in XChat

static char CmdStackText[512];
static char *CmdStackPtr = NULL;

static int ContextStackSP = 0;
static xchat_context *ContextStack[16];
static int RandomType = 1;
static int CharCounter = 0;
static int PMAlerts = 0;
static int PMAlertsEvenWithoutFocus = 0;
static int PMAlertsEvenWithSameNetwork = 0;
static int QuietOnEvents = 0;

static int DisableShowNetworkOnJoin=0;
static int NeedSpaceBetweenX5Font = 0;
static char PesterchumChanHook[256] = "spark pestersay";
static char PesterchumColor[64] = "0,0,0";

static xchat_hook *SpawnHook[64] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static char JanusLinkbots[4][32] = {"IotaIRC","IotaIRC_","",""};

// 02 16 1f 16 16 0f 02 03 30 30 03 30 30
static const char SparkEncryptPrefix[] = {2,0x16,0x1f,0x16,0x16,0x0f,2,3,'0','0',3,'0','0',0};
static char SparklesUser[64];

static char *Rainbow[]={"04","08","09","12","13"};

static void INIConfigHandler(const char *Group, const char *Item, const char *Value);
static int ParseINI(FILE *File, void (*Handler)(const char *Group, const char *Item, const char *Value));

enum OnEventFlags {
  OEF_ENABLED = 1,
  OEF_TEMPORARY = 2,
  OEF_SAVE = 4,
  OEF_HIGH_PRIORITY = 8,
};

struct OnEventInfo {
  int  Slot;
  int  Flags; // 1 = enabled, 2 = temporary, 4 = save
  char EventName[32];
  char Match[10][80];
  char Response[500];
  xchat_hook *Hook;
};

#define ONEVENTS_SIZE 30
struct OnEventInfo *OnEventInfos[ONEVENTS_SIZE];

enum ConfigTypes {
  CONFIG_STRING,
  CONFIG_BOOLEAN,
  CONFIG_INTEGER,
};

struct ConfigItem {
  char *Group;
  char *Item;
  void *Data;
  char Type;
  short Len;
} ConfigOptions[] = {
  {"General", "RandomType", &RandomType, CONFIG_INTEGER, 0},
  {"General", "TextEditor", &TextEditor, CONFIG_STRING, 512},
  {"General", "QuietOnEvents", &QuietOnEvents, CONFIG_BOOLEAN, 0},
  {"Automatic", "ForceUTF8", &ForceUTF8, CONFIG_BOOLEAN, 0},
  {"Automatic", "CharCounter", &CharCounter, CONFIG_BOOLEAN, 0},
  {"Automatic", "PMAlerts", &PMAlerts, CONFIG_BOOLEAN, 0},
  {"Automatic", "PMAlertsEvenWithoutFocus", &PMAlertsEvenWithoutFocus, CONFIG_BOOLEAN, 0},
  {"Automatic", "PMAlertsEvenSameNetwork", &PMAlertsEvenWithSameNetwork, CONFIG_BOOLEAN, 0},
  {"Automatic", "RejoinOnKick", &RejoinKick, CONFIG_BOOLEAN, 0},
  {"Automatic", "JoinOnInvite", &JoinOnInvite, CONFIG_BOOLEAN, 0},
  {"Automatic", "DisableAutoIdent", &DisableAutoIdent, CONFIG_BOOLEAN, 0},
  {"Automatic", "DisableAutoNickColorReset", &DisableAutoNickDeblue, CONFIG_BOOLEAN, 0},
  {"Automatic", "DisableAutoGhost", &DisableAutoGhost, CONFIG_BOOLEAN, 0},
  {"Automatic", "DisableAutoGhostChainBreak", &DisableAutoGhostChainBreak, CONFIG_BOOLEAN, 0},
  {"Automatic", "Activity2Focus", &Activity2Focus, CONFIG_BOOLEAN, 0},
  {"Automatic", "DisableNetworkSayer", &DisableShowNetworkOnJoin, CONFIG_BOOLEAN, 0},
  {"Automatic", "DisableHighlights", &EatHighlights, CONFIG_BOOLEAN, 0},
//  {"Automatic", "MoveNotifyToServerTab", &MoveNotifyToServer, CONFIG_BOOLEAN, 0},
//  {"Automatic", "MoveServicesToServerTab", &MoveServicesToServer, CONFIG_BOOLEAN, 0},
  {"PrettyJanus", "Disabled", &DisablePrettyJanus, CONFIG_BOOLEAN, 0},
  {"PrettyJanus", "Nick0", JanusLinkbots[0], CONFIG_STRING, 32},
  {"PrettyJanus", "Nick1", JanusLinkbots[1], CONFIG_STRING, 32},
  {"PrettyJanus", "Nick2", JanusLinkbots[2], CONFIG_STRING, 32},
  {"PrettyJanus", "Nick3", JanusLinkbots[3], CONFIG_STRING, 32},
  {"Pesterchum", "Disabled", &DisablePesterchum, CONFIG_BOOLEAN, 0},
  {"Pesterchum", "ChannelCommand", PesterchumChanHook, CONFIG_STRING, 512},
  {"Pesterchum", "Color", PesterchumColor, CONFIG_STRING, 64},
  {NULL}, // <-- end marker
};

void strlcpy(char *Destination, const char *Source, int MaxLength) {
  // MaxLength is directly from sizeof() so it includes the zero
  int SourceLen = strlen(Source);
  if((SourceLen+1) < MaxLength)
    MaxLength = SourceLen + 1;
  memcpy(Destination, Source, MaxLength-1);
  Destination[MaxLength-1] = 0;
}

int memcasecmp(const char *Text1, const char *Text2, int Length) {
  for(;Length;Length--)
    if(tolower(*(Text1++)) != tolower(*(Text2++)))
      return 1;
  return 0;
}

static int isgraph2(char k) { // unicode version
  if(!isgraph(k)) return 0;
  unsigned char k2 = (unsigned char)k;
  return k2<0x80||k2>0xbf;
}

static char *ReadTextFile(const char *Name) {
  FILE *File = fopen(Name,"rb");
  if(!File) return NULL;
  fseek(File, 0, SEEK_END);
  long FileSize = ftell(File);
  rewind(File);
  char *Buffer = (char*)malloc(sizeof(char)*FileSize+1);
  if(Buffer == NULL) {
    fclose(File);
    return NULL;
  }
  if(FileSize != fread(Buffer,1,FileSize,File)) {
    fclose(File);
    return NULL;
  }
  fclose(File);
  Buffer[FileSize] = 0;
  return Buffer;
}

static int OETextMatch(char *Inputs[], const char *MatchTo) {
  int CaseSensitive = 0, Negate = 0, Decision = 0;
  int Wild = 0, InputNum, SpecialInput = 0;
  const char *Seek = MatchTo+1;
  char *Stripped;
  InputNum = MatchTo[0] - '0';
  char Input[800];
  if(InputNum < 0 || InputNum > 4)
    return 0;
  if(InputNum == 0) { // special set of inputs
    SpecialInput = 1;
    const char *CopyFrom = NULL;
    switch(*(Seek++)) {
      case 'c': CopyFrom = xchat_get_info(ph, "channel"); break;
      case 'n': CopyFrom = xchat_get_info(ph, "nick"); break;
      case 'N': CopyFrom = xchat_get_info(ph, "network"); break;
      case 's': CopyFrom = xchat_get_info(ph, "server"); break;
      case 'w': CopyFrom = xchat_get_info(ph, "win_status"); break;
    }
    if(!CopyFrom) CopyFrom = "";
    strlcpy(Input, CopyFrom, 800);
  } else {
    Stripped = xchat_strip(ph, Inputs[InputNum], -1, 1 | 2);
    strlcpy(Input, Stripped, 800);
    xchat_free(ph, Stripped);
  }
  char Match[strlen(MatchTo)+1];
  if(*Seek == '!') {
    Negate = 1;
    Seek++;
  }
  if(*(Seek++) != '=')
    return 0;
  if(*Seek == '=') { // two equals signs, turn on case sensitive mode
    CaseSensitive = 1;
    Seek++;
  }
  if(*Seek == '*') { // left wildcard
    Wild |= 1;
    Seek++;
  }
  strcpy(Match, Seek);
  char *End = strrchr(Match, 0);
  if(End[-1] == '*') { // right wildcard
    Wild |= 2;
    End[-1] = 0;
  }
  if(!CaseSensitive) { // make input all lowercase if not case sensitive
    char *ToLower;
    for(ToLower = Input; *ToLower; ToLower++)
      *ToLower = tolower(*ToLower);
    for(ToLower = Match; *ToLower; ToLower++)
      *ToLower = tolower(*ToLower);
  }
  switch(Wild) {
    case 0: // @
      Decision = !strcasecmp(Input, Match);
      break;
    case 1: // *@
      End = strrchr(Input, 0);
      End -= strlen(Match);
      Decision = memcasecmp(End, Match, strlen(Match));
      break;
    case 2: // @*
      Decision = memcasecmp(Input, Match, strlen(Match));
      break;
    case 3: // *@*
      Decision = strstr(Input, Match) != NULL;
      break;
  }
  if(Negate)
    Decision ^= 1;
  return Decision;
}

static void TextInterpolate(char *Out, const char *In, char Prefix, const char *ReplaceThis, const char *ReplaceWith[]) {
  while(*In) {
    if(*In != Prefix)
      *(Out++) = *(In++);
    else {
      In++;
      char *Find = strchr(ReplaceThis, *(In++));
      if(Find) {
        int This = Find - ReplaceThis;
        strcpy(Out, ReplaceWith[This]);
        Out += strlen(ReplaceWith[This]);
      } else {
        *(Out++) = Prefix;
        *(Out++) = In[-1];
      }
    }
  }
  *Out = 0;
}

struct SpawnInfo {
  int Slot;
  int TimeAmount;
  char Command[1024];
  xchat_context *Context;
};

static int spawntimer_cb(void *userdata) {
  struct SpawnInfo *Spawn = (struct SpawnInfo *)userdata;
  if(Spawn) {
    if(xchat_set_context(ph,Spawn->Context))
      xchat_command(ph, Spawn->Command);
    else
      xchat_printf(ph, "%sContext switch failed for spawn %i \n", SparklesUser, Spawn->Slot);
    SpawnHook[Spawn->Slot]=NULL;
    free(Spawn);
  }
  return 0;
}

static void UpdateCharCounter() {
  if(!strlen(xchat_get_info(ph, "host")))
    return;

  int Len = strlen(xchat_get_info(ph, "inputbox"));
  xchat_context *Context = xchat_get_context(ph);
  int Id;
  xchat_get_prefs(ph, "id", NULL, &Id);

  xchat_list *list = xchat_list_get(ph, "channels");
  if(list) {
    while(xchat_list_next(ph, list)) {
      if(xchat_list_int(ph, list, "type")==1 && xchat_list_int(ph, list, "id")==Id) { // server
        if(xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context"))) {
          if(Len)
            xchat_commandf(ph, "settab %-3i %s", Len, xchat_get_info(ph, "channel"));
          else
            xchat_commandf(ph, "settab %s", xchat_get_info(ph, "channel"));
        }
      }
    }
    xchat_list_free(ph, list);
  }
  xchat_set_context(ph, Context);
}

static int timer_cb(void *userdata) {
  if(CharCounter)
    UpdateCharCounter();
  return 1;
}

static int charcounter_cb(char *word[], void *userdata) {
  if(CharCounter)
    UpdateCharCounter();
  return XCHAT_EAT_NONE;
}

static int on_event_cb(char *word[], void *userdata) {
  struct OnEventInfo *Info = (struct OnEventInfo*)userdata;
  if(!Info)
    return XCHAT_EAT_NONE;
  int i;
  for(i=0;Info->Match[i][0];i++)
    if(!OETextMatch(word, Info->Match[i]))
      return XCHAT_EAT_NONE;

  char *Word1 = xchat_strip(ph, word[1], -1, 3);
  char *Word2 = xchat_strip(ph, word[2], -1, 3);
  char *Word3 = xchat_strip(ph, word[3], -1, 3);
  char *Word4 = xchat_strip(ph, word[4], -1, 3);
  char Buffer[2048];
  const char *ReplaceWith[] = {xchat_get_info(ph, "nick"), xchat_get_info(ph, "channel"), Word1, Word2, Word3, Word4};
  TextInterpolate(Buffer, Info->Response, '$', "nc1234", ReplaceWith);
  xchat_free(ph, Word1);
  xchat_free(ph, Word2);
  xchat_free(ph, Word3);
  xchat_free(ph, Word4);

  xchat_command(ph, Buffer);
  if(Info->Flags & OEF_TEMPORARY)
    xchat_commandf(ph, "spark onevent delete %i", Info->Slot);

  return XCHAT_EAT_NONE;
}

static int privatemessage_cb(char *word[], void *userdata) {
  if(PMAlerts) {
    if(!PMAlertsEvenWithoutFocus) {
/* well apparently this is in xchat already
      HWND FGWindow = GetForegroundWindow();
      HWND XChat = (HWND)xchat_get_info(ph, "win_ptr");
      if(FGWindow != XChat)
        return XCHAT_EAT_NONE;
*/
      if(strcasecmp(xchat_get_info(ph, "win_status"), "active"))
        return XCHAT_EAT_NONE;
    }
    xchat_context *Focused = xchat_find_context(ph, NULL, NULL);
    xchat_context *This = xchat_get_context(ph);
    if(Focused == This)
      return XCHAT_EAT_NONE;
    char Network[32];
    strlcpy(Network, xchat_get_info(ph, "network"), 32);
    xchat_set_context(ph, Focused);
    if(PMAlertsEvenWithSameNetwork || !xchat_get_info(ph, "network") || strcasecmp(Network, xchat_get_info(ph, "network")))
      xchat_printf(ph, "----\t\x03" "31PM on %s with %s: %s", Network, word[1], word[2]);
    xchat_set_context(ph, This);
  }
  return XCHAT_EAT_NONE;
}

int MakeDirectory(const char *Path) {
#ifdef _WIN32
  return CreateDirectory(Path, NULL);
#else
  return !mkdir(Path, 0700);
#endif
}

char *FindCloserPointer(char *A, char *B) {
  if(!A) // doesn't matter if B is NULL too, it'll just return the NULL
    return B;
  if(!B || A < B)
    return A;
  return B;
}

int CreateDirectoriesForPath(const char *Folders) {
  char Temp[strlen(Folders)+1];
  strcpy(Temp, Folders);
  struct stat st = {0};

  char *Try = Temp;
  if(Try[1] == ':' && Try[2] == '\\') // ignore drive names
    Try = FindCloserPointer(strchr(Try+3, '/'), strchr(Try+3, '\\'));

  while(Try) {
    char Restore = *Try;
    *Try = 0;
    if(stat(Temp, &st) == -1) {
      MakeDirectory(Temp);
      if(stat(Temp, &st) == -1)
        return 0;
    }
    *Try = Restore;
    Try = FindCloserPointer(strchr(Try+1, '/'), strchr(Try+1, '\\'));
  }
  return 1;
}

static unsigned int MZXRand(unsigned long long range) {
   static unsigned long long seed = 0;
   unsigned long long value;
   // If the seed is 0, initialise it with time and clock
   if(seed == 0)
     seed = time(NULL) + clock();
   seed = seed * 1664525 + 1013904223;
   value = (seed & 0xFFFFFFFF) * range / 0xFFFFFFFF;
   return (unsigned int)value;
}

static int RepeatRand(int Max) {
  Max = abs(Max);
  int Mask = 1;
  while(Mask < (Max-1))
    Mask = (Mask<<1) | 1;
  int Try;
  while(1) {
    if(RandomType!=3)
      Try = rand()&Mask;
    else
      Try = MZXRand(Mask+1);
    if(Try > (Max-1)) continue;
    break;
  }
  return Try;
}

static int rand2(int max) {
  if(max <= 0) return 0;
  switch(RandomType) {
    default:
      return rand()%max;
    case 1:
      return RepeatRand(max);
    case 2:
      return MZXRand(max);
    case 3:
      return RepeatRand(max);
  }
}

static int IsPesterchum() {
  const char *Net = xchat_get_info(ph,"network");
  if(Net == NULL) return 0;
  return !strcasecmp(Net, PESTERCHUM_NETWORK);
}
static int IsChannel() {
  const char *Chan = xchat_get_info(ph,"channel");
  if(Chan == NULL) return 0;
  return *Chan == '#';
}

static int MyNickCmp(char *N1, char *N2) {
   char *StripName;
   StripName = xchat_strip(ph, N1, -1, 3);
   if(StripName == NULL)
     return 0;
   if(!strcasecmp(StripName, N2)) {
     xchat_free(ph, StripName);
     return 1;
   }
   xchat_free(ph, StripName);
   return 0;
}

struct RandomReplaceList {
  char In; int Amount; char *Out[8];
};

struct RandomReplaceList AccentList[] = {
  {'A',6,{"À","Á","Â","Ã","Ä","Å"}},
  {'a',6,{"à","á","â","ã","ä","å"}},
  {'E',4,{"È","É","Ê","Ë"}},
  {'e',6,{"è","é","ê","ë","e","e"}},
  {'O',6,{"Ò","Ó","Ô","Õ","Ö","Ø"}},
  {'o',6,{"ò","ó","ô","õ","ö","ø"}},
  {'U',4,{"Ù","Ú","Û","Ü"}},
  {'u',4,{"ù","ú","û","ü"}},
  {'I',4,{"Ì","Í","Î","Ï"}},
  {'i',4,{"ì","í","î","ï"}},
  {0}
};

static char *AccentFilter(char *Output, char *Input) {
  char *Poke = Output, *Peek = Input;
  while(*Peek) {
    char k = *(Peek++);
    char *Replacement = NULL;
    if(k == 'N') Replacement = "Ñ";
    if(k == 'n') Replacement = "ñ";
    if(k == 'S') Replacement = "Š";
    if(k == 's') Replacement = "š";
    if(k == 'Z') Replacement = "Ž";
    if(k == 'y') Replacement = "ý";
    if(k == 'Y') Replacement = "Ý";
    if(k == 'D') Replacement = "Ð";
    if(k == 'C') Replacement = "Ç";
    if(k == 'c') Replacement = "ç";
    if(k == 'B') Replacement = "ß";
    int i;
    for(i=0;AccentList[i].In;i++)
      if(k==AccentList[i].In) {
        int j = rand2(AccentList[i].Amount);
        Replacement = AccentList[i].Out[j];
        break;
      }
    if(Replacement) {
      strcpy(Poke, Replacement);
      Poke += strlen(Replacement);
    } else
      *(Poke++) = k;
  }
  *Poke = 0;
  return Output;
}

struct ReplaceList {
  char *In; char *Out; int Mode;
};

struct ReplaceList SellyList[] = {
  {"you", "u", 0}, {"your", "ur", 0}, {"you're", "ur", 0}, {"know", "kno", 0}, {"threw", "thru", 0}, {"through", "thru", 0},
  {"where", "where", 0}, {"there", "there", 0}, {"place", "plase", 0}, {"really", "reely", 0},
  {"yone", "1", 0}, {" one", " 1", 0}, {"yeah", "ya", 0}, {"mean", "meen", 0}, {"because", "cuz", 0},
  {"'cause", "cuz", 0}, {"you", "u", 0}, {"i'm", "im", 0}, {"though", "tho", 0}, {"weren't", "wernt", 0}, {"people", "ppl", 0},
  {"weird", "wierd", 0}, {"i'm going to", "ima", 0}, {"going to", "gunna", 0}, {"gonna", "gunna", 0}, {"until", "till", 0},
  {"when", "wen", 0}, {"like", "liek", 0}, {"person", "persin", 0}, {"here", "heer", 0}, {"probably", "probly", 0}, {"should", "shud", 0},
  {"are you", "ru", 0}, {"what", "wat", 0}, {"tried", "tryed", 0}, {"ould", "ud", 0}, {"bye", "bai", 0}, {" are ", " r ", 0}, {"really", "rlly", 0},
  {"please", "plz", 0}, {"'t", "t", 0}, {"'ll", "ll", 0}, {"'s", "s", 0}, {"'re", "re", 0}, {"'ve", "ve", 0}, {"'d", "d", 0},

  {"why", "y", 1}, {"was", "wuz", 1}, {"please", "plz", 1}, {" the ", " teh ", 1}, {"more", "moar", 1}, {"with", "wif", 1},
  {"my", "mai", 1}, // gud
  {NULL, NULL, 0}
};

static char *SellyFilter(char *Output, char *Input, int Mode) {
  char *Poke = Output;
  char *Peek = Input;
  void Replace(char *Old, char *New, int Mode) {
    int i=0;
    if(Peek==Input)
      if(Old[0]==' ') {
        Old++;
        New++;
      }
    for(i=0;Old[i];i++)
      if(Old[i] != tolower(Peek[i]))
        return;
    i=0;
    memcpy(Poke, New, strlen(New));
    Peek+=strlen(Old)-1;
    Poke+=strlen(New)-1;
  }
  while(*Peek) {
    if((Peek == Input) || (Peek[-1]!=':' && Peek[-1]!=';'))
      *Poke = tolower(*Peek);
    else
      *Poke = *Peek;
    int i=0;
    for(i=0;SellyList[i].In != NULL;i++)
      if(Mode >= (SellyList[i].Mode & 3))
        Replace(SellyList[i].In, SellyList[i].Out, 0);
    Poke++; Peek++;
  }
  *Poke = 0;
  return Output;
}

static char *ScarletFilter(char *Output, char *Input) {
  char *Poke = Output;
  char *Peek = Input;
  void Replace(char *Old, char *New, int Mode) {
    int i=0, WasUpper=0;
    if(isupper(*Peek))
      WasUpper=1;
    if(Peek==Input)
      if(Old[0]==' ') {
        Old++;
        New++;
      }
    for(i=0;Old[i];i++)
      if(Old[i] != tolower(Peek[i]))
        return;
    int AllCaps = 1;
    for(i=0;Old[i];i++) {
      if(isalpha(Peek[i]) && islower(Peek[i])) {
        AllCaps = 0;
        break;
      }
    }
    i=0;
    memcpy(Poke, New, strlen(New));
    if(AllCaps)
      for(i=0;i<strlen(New);i++)
        Poke[i] = toupper(Poke[i]);
    if(WasUpper)
      *Poke = toupper(*Poke);
    Peek+=strlen(Old)-1;
    Poke+=strlen(New)-1;
  }
  while(*Peek) {
    *Poke = *Peek;
    Replace("really", "rilly", 0);
    Replace("stupid", "stoopid", 0);
    Replace("because", "cuz", 0);
    Replace("'cause", "cuz", 0);
    Replace("in a", "inna", 0);
    Replace("suppose", "s'pose", 0);
    Replace("nope!", "nuh-uh!", 0);
    Replace("uh huh", "ah-hah", 0);
    Replace("some", "sum", 0);
    Replace(" and ", " an' ", 0);
    Replace("was", "wuz", 0);
    Replace("is that", "izzat", 0);
    Replace("hello", "hallo", 0);
    Replace("ever", "eva", 0);
    Replace("'kay", "a'kay", 0);
    Replace("okay", "a'kay", 0);
    Replace("oh no!", "OH NOES!", 0);
    Replace("just ", "jus' ", 0);
    Poke++; Peek++;
  }
  *Poke = 0;
  return Output;
}

static char *CustomReplace(char *Output, char *Input, char *List) {
  char *Poke = Output;
  char *Peek = Input;

  char NewList[768];
  strcpy(NewList, List);
  char *Words[100];
  int NumWords = 0;
  char *WordPtr = NewList;
  char *NextPtr = NULL;

  int KeepCaps = 1;

  while(1) {
    NextPtr = strstr(WordPtr, "||");
    if(NextPtr == NULL) {
//      xchat_printf(ph, "%s\n", WordPtr);
      Words[NumWords++]=WordPtr;
      break;
    } else {
      *NextPtr = 0;
      Words[NumWords++] = WordPtr;
//      xchat_printf(ph, "%s\n", WordPtr);
      WordPtr = NextPtr + 2;
    }
  }

  void Replace(char *Old, char *New, int Mode) {
    int i=0, WasUpper=0;
    if(isupper(*Peek))
      WasUpper=1;
    if(Peek==Input && Old[0]==' ') {
      Old++;
      New++;
    }
    for(i=0;Old[i];i++)
      if(Old[i] != tolower(Peek[i]))
        return;
    int AllCaps = 1;
    for(i=0;Old[i];i++) {
      if(isalpha(Peek[i]) && islower(Peek[i])) {
        AllCaps = 0;
        break;
      }
    }
    i=0;
    memcpy(Poke, New, strlen(New));
    if(AllCaps && KeepCaps)
      for(i=0;i<strlen(New);i++)
        Poke[i] = toupper(Poke[i]);
    if(WasUpper)
      *Poke = toupper(*Poke);
    Peek+=strlen(Old)-1;
    Poke+=strlen(New)-1;
  }
  char *ReplaceTo = NULL; // word to replace to
  while(*Peek) {
    *Poke = *Peek;
    KeepCaps = 1;
    int i;
    for(i=0;i<NumWords;i++) {
//      xchat_printf(ph, "%s - ", Words[i]);
      if(Words[i][0] == '>') // change to
        ReplaceTo = Words[i]+1;
      if(Words[i][0] == '<') // change from
        if(ReplaceTo!=NULL)
          Replace(Words[i]+1, ReplaceTo, 0);
      if(Words[i][0] == '^') { // set flags
        if(Words[i][1] == 'K' && Words[i][2] == 'C')
          KeepCaps ^= 1;
      }
    }
    Poke++; Peek++;
  }
  *Poke = 0;
  return Output;
}

static void ShuffleChars(char *Poke, char *Peek, int Swaps, int Flags) {
  strcpy(Poke,Peek);
  char *End = Poke + strlen(Poke);
  int i,EndNow = 0;
  char *Next;
  while(Poke <= End && !EndNow) {
    Next = Poke;
    while(isalpha(*Next))
      Next++;
    int WordLen = Next - Poke;
    if(WordLen>=2) { // must be at least 4 letters
      for(i=0;i<Swaps;i++) {
        int Max = WordLen - 2;
        int A=(rand2(Max)) + 1;
        int B=(rand2(Max)) + 1;
        char t;
        t = Poke[A];
        Poke[A] = Poke[B];
        Poke[B] = t;
      }
    }
    Poke = Next; // next word
    while(!isalpha(*Poke)) {
      if(*Poke == 0) { // no more words?
        EndNow = 1;
        break;
      }
      Poke++;
    }
  }
}

// used for /spark atloop
static int AttributeLoop(char *Out, char *In, char *Loop) {
  int Caps = 0;
  int i = 0; // input index
  int o = 0; // output index
  int s = 0; // script
  int LP = 0; // loop position

  int RainbowIndex = 0;

  int EndI = strlen(In);
//  int EndL = strlen(Loop);
  int LoopTimes = 0;

  for(;i<EndI;) {
    switch(Loop[s++]) {
      case 'c':
        Out[o++]=0x03;
        if(tolower(Loop[s]) != 'g') {
          Out[o++]=Loop[s++];
          Out[o++]=Loop[s++];
        }
        else {
          Out[o++]=Rainbow[RainbowIndex][0];
          Out[o++]=Rainbow[RainbowIndex][1];
          if(Loop[s]!='G')
            RainbowIndex++;
          if(RainbowIndex > 4) RainbowIndex = 0;
          s++;
        }
        if(Loop[s] == ',') {
          Out[o++]=Loop[s++];
          if(tolower(Loop[s]) != 'g') {
            Out[o++]=Loop[s++];
            Out[o++]=Loop[s++];
          }
          else {
            Out[o++]=Rainbow[RainbowIndex][0];
            Out[o++]=Rainbow[RainbowIndex][1];
            if(Loop[s]!='G')
              RainbowIndex++;
            if(RainbowIndex > 4) RainbowIndex = 0;
            s++;
          }
        }
        break;
      case 'u': Out[o++]=0x1f; break;
      case 'b': Out[o++]=0x02; break;
      case 'i': Out[o++]=0x1d; break;
      case 'R': Out[o++]=0x16; break;
      case 'p': Out[o++]=0x0f; break;
      case '_': Out[o++]=' '; break;

      case '\'': Out[o++]=Loop[s++]; break;
      case '*':
        switch(Caps) {
          case 0: Out[o++]=In[i++]; break;
          case 1: Out[o++]=toupper(In[i++]); break;
          case 2: Out[o++]=tolower(In[i++]); break;
          case 3: Out[o++]=(rand()&1)?toupper(In[i++]):tolower(In[i++]); break;
        }
        break;

      case 'C':
        switch(Loop[s++]) {
          case 'N': Caps=0; break;
          case 'U': Caps=1; break;
          case 'L': Caps=2; break;
          case 'R': Caps=3; break;
        }
        break;

      case '`':
        switch(Loop[s++]) {
          case '?':
            while(!(isgraph2(In[i]) || (In[i]==' ')) && i<EndI)
              Out[o++]=In[i++];
            break;
          case 'c':
            Out[o++]=0x03; break;
            break;
        }
        break;


      case '?':
        while(!isgraph2(In[i]) && i<EndI)
          Out[o++]=In[i++];
        switch(Caps) {
          case 0: Out[o++]=In[i++]; break;
          case 1: Out[o++]=toupper(In[i++]); break;
          case 2: Out[o++]=tolower(In[i++]); break;
          case 3: Out[o++]=(rand()&1)?toupper(In[i++]):tolower(In[i++]); break;
        }
        break;

      case '|': LP=s; break;
      case 0: s=LP; LoopTimes++; if(LoopTimes > 1000) return 1; break;
    }
  }
  Out[o]=0;
  return 0;
}

static char *Adj(int Amount, char *Word, ...);
static char *TrollFilter(char *Output, char *Input, char *Troll) {
  if(Troll[0] == 0) return "???";
  if(Troll[1] == 0) return "???";
 
  char *Poke = Output;
  char *Peek = Input;
  int j;
  void LowerAll() {
    int i;
    for(i=0;Output[i];i++)
      Output[i]=tolower(Output[i]);
  }
  void UpperAll() {
    int i;
    for(i=0;Output[i];i++)
      Output[i]=toupper(Output[i]);
  }
  void Replace(char *Old, char *New, int Mode) {
    int i=0;
    if(Peek==Input && Old[0]==' ') {
      Old++;
      New++;
    }
    for(i=0;Old[i];i++)
      if(Old[i] != tolower(Peek[i]))
        return;
    i=0;
    memcpy(Poke, New, strlen(New));
    Peek+=strlen(Old)-1;
    Poke+=strlen(New)-1;
  }
  int TrollNum = 0;
  switch(tolower(Troll[0])) {
    case 'a': TrollNum|=0;  break;
    case 't': TrollNum|=4;  break;
    case 'g': TrollNum|=8;  break;
    case 'c': TrollNum|=12; break;
  }
  switch(tolower(Troll[1])) {
    case 'a': TrollNum|=0; break;
    case 't': TrollNum|=1; break;
    case 'g': TrollNum|=2; break;
    case 'c': TrollNum|=3; break;
  }

  if(Troll[0]=='U'&&Troll[1]=='U') {
    while(*Peek) {
      *Poke = *Peek;
      if(tolower(Peek[0]) == 'u') *Poke = 'U';
      else *Poke = tolower(*Peek);
      Poke++; Peek++;
    }
  } else if(Troll[0]=='u'&&Troll[1]=='u') {
    while(*Peek) {
      *Poke = *Peek;
      if(toupper(Peek[0]) == 'U') *Poke = 'u';
      else *Poke = toupper(*Peek);
      Poke++; Peek++;
    }
  } else if(tolower(Troll[0])=='l'&&tolower(Troll[1])=='p') {
    while(*Peek) {
      *Poke = *Peek;
      if(tolower(Peek[0]) == 'a') *Poke = '4';
      if(tolower(Peek[0]) == 'i') *Poke = '1';
      if(tolower(Peek[0]) == 'e') *Poke = '3';
      Poke++; Peek++;
    }
    LowerAll();
  } else if(tolower(Troll[0])=='m'&&tolower(Troll[1])=='c') {
    while(*Peek) {
      *Poke = toupper(*Peek);
      if(tolower(Peek[0]) == 'a') *Poke = '4';
      if(tolower(Peek[0]) == 's') *Poke = '5';
      if(tolower(Peek[0]) == 'o') *Poke = '0';
      if(tolower(Peek[0]) == 't') *Poke = '7';
      if(tolower(Peek[0]) == 'b') *Poke = '8';
      if(tolower(Peek[0]) == 'i') *Poke = '1';
      if(tolower(Peek[0]) == 'e') *Poke = '3';
      Poke++; Peek++;
    }
  } else if(tolower(Troll[0])=='k'&&tolower(Troll[1])=='v') {
    while(*Peek) {
      *Poke = *Peek;
      if(tolower(Peek[0]) == 'b') *Poke = '6';
      if(tolower(Peek[0]) == 'o') *Poke = '9';
      Poke++; Peek++;
    }
  } else if(tolower(Troll[0])=='m'&&tolower(Troll[1])=='l') {
      strcpy(Poke, Adj(8, "(=`ω´=) ","(^•ω•^) ","(=^•^=) ","(=^•ω•^=) ","(^•o•^) ","(=^-ω-^=) ","(=^ω^=) ","(=TωT=) "));
      Poke = strchr(Poke, 0);
      if(!Poke) return "???";
      while(*Peek) {
        *Poke = *Peek;
        Replace("ee", "33", 0);
        Replace("pause", "pawse", 0);
        Replace(" per", " purr", 0);
        Replace(" for", " fur", 0);
        Replace(" pos", " paws", 0);
        Replace(" pro", " purro", 0);
        Replace(":3", ":33", 0);
        Replace("x3", "X33", 0);
        Replace(":D", ":DD", 0);
        Replace(":)", ":))", 0);
        Replace(":(", ":((", 0);
        Replace("transparent", "transpurrent", 0);
        Replace("hypocrite", "hypurrcrite", 0);
        Poke++; Peek++;
      }
  } else if(tolower(Troll[0])=='a'&&tolower(Troll[1])=='s') {
    while(*Peek) {
      *Poke = *Peek;
      if(tolower(Peek[0]) == 'b') *Poke = '8';
      Poke++; Peek++;
    }
  } else if(tolower(Troll[0])=='p'&&tolower(Troll[1])=='m') {
    while(*Peek) {
      *Poke = *Peek;
      if(tolower(Peek[0]) == 'o') *(++Poke) = '+';
      Replace(" plus", " +", 0);
      Poke++; Peek++;
    }
  } else if(tolower(Troll[0])=='m'&&tolower(Troll[1])=='p') {
    while(*Peek) {
      *Poke = *Peek;
      Replace("h", ")(", 0);
      Replace("H", ")(", 0);
      Replace(":", "38", 0);
      if(*Peek == 'E') {
        *(Poke++) = '-';
        *Poke = 'E';
      }
      Poke++; Peek++;
    }
  } else if(tolower(Troll[0])=='k'&&tolower(Troll[1])=='m') {
    *(Poke++) = 0x02;
    *(Poke++) = 0x03;
    *(Poke++) = '1';
    *(Poke++) = '3';
    *(Poke++) = ',';
    *(Poke++) = '0';
    *(Poke++) = '1';
    while(*Peek) {
      *Poke = toupper(*Peek);
      Poke++; Peek++;
    }
  } else if(tolower(Troll[0])=='h'&&tolower(Troll[1])=='z') {
      strcpy(Poke, "8=D < ");
      Poke += 6;

      while(*Peek) {
        *Poke = *Peek;
        Replace("strong","STRONG",0);
        Replace("loo","100",0);
        Replace("lue ","100 ",0);
        Replace("ool","001",0);
        if(tolower(Peek[0]) == 'x') *Poke = '%';
        Replace("ks","%",0);
        Poke++; Peek++;
      }
  } else if(tolower(Troll[0])=='c'&&tolower(Troll[1])=='r') {
    while(*Peek) {
      *Poke = *Peek;
      if(tolower(Peek[0]) == 'w') *(++Poke) = 'v';
      if(tolower(Peek[0]) == 'v') *(++Poke) = 'w';
      if(Peek[0] == 'B' && isupper(Peek[-1])) *(++Poke) = '8';
      Replace("ing ","in ",0);
      Poke++;
      Peek++;
    }
  } else if(tolower(Troll[0])=='d'&&tolower(Troll[1])=='m') {
    while(*Peek)
      *(Poke++) = *(Peek++);
    UpperAll();
  } else if(tolower(Troll[0])=='r'&&tolower(Troll[1])=='n') {
    while(*Peek) {
      *(Poke++) = *(Peek++);
      if(tolower(Peek[0]) == 'i') *(++Poke) = '1';
    }
    LowerAll();
  } else switch(TrollNum) {
    case 0:  // aa
      while(*Peek) {
        *(Poke++) = *(Peek++);
        if(tolower(Peek[-1]) == 'O') Poke[-1] = '0';
      }
      LowerAll();
      break;
    case 1:  // at
      while(*Peek) {
        if(isupper(*Peek))
          *Poke = tolower(*Peek);
        else
          *Poke = toupper(*Peek);
        if(Peek == Input)
          *Poke = tolower(*Peek);
        if(*Peek == '.') *Poke = ',';
        Replace(":", "}:", 0);
        Poke++; Peek++;
      }
      break;
    case 2:  // ag
      while(*Peek) {
        *(Poke++) = *(Peek++);
        if(tolower(Peek[-1]) == 'b') Poke[-1] = '8';
        Replace("ate", "8", 0);
        Replace("ait", "8", 0);
        Replace(":", "::::", 0);
      }
      break;
    case 3:  // ac, http://mspaintadventures.com/?s=6&p=004062
      strcpy(Poke, ":33 < ");
      Poke += 6;
      while(*Peek) {
        *Poke = *Peek;
        Replace("ee", "33", 0);
        Replace("pause", "pawse", 0);
        Replace(" per", " purr", 0);
        Replace(" for", " fur", 0);
        Replace(" pos", " paws", 0);
        Replace(" pro", " purro", 0);
        Replace(":3", ":33", 0);
        Replace("x3", "X33", 0);
        Replace(":D", ":DD", 0);
        Replace(":)", ":))", 0);
        Replace(":(", ":((", 0);
        Replace("transparent", "transpurrent", 0);
        Replace("hypocrite", "hypurrcrite", 0);
// amewsment, purrk, meowscular, purrtend, oppurtunity, prepawsterous, pawsture, accomeowdate
        Poke++; Peek++;
      }
      LowerAll();
      break;
    case 4:  // ta
      while(*Peek) {
        *Poke = *Peek;
        if(tolower(Peek[0]) == 's') *Poke = '2';
        if(tolower(Peek[0]) == 'i') *(++Poke) = 'i';
        Poke++; Peek++;
      }
      Replace(" to"," two",0);
      Replace(" too "," two ",0);
      LowerAll();
      break;
    case 5:  // tt
      // not a troll, rose
      while(*Peek)
        *(Poke++) = *(Peek++);
      break;
    case 6:  // tg
      // not a troll, dave
      while(*Peek)
        *(Poke++) = *(Peek++);
      break;
    case 7:  // tc, gamzee
      j=0;
      while(*Peek) {
        *Poke = ((j&1)?tolower(*Peek):toupper(*Peek));
        if(isgraph2(*Poke)) j++;
        if(*Poke == 'O' && Poke[-1]==':') *Poke = 'o';
        Poke++; Peek++;
      }
      break;
    case 8:  // ga
      while(*Peek) {
        *Poke = *Peek;
        if(Peek==Input || (tolower(Peek[-1]) == ' ')) *Poke = toupper(*Peek);
        Poke++; Peek++;
      }
      break;
    case 9:  // gt
      // not a troll, john
      while(*Peek)
        *(Poke++) = *(Peek++);
      break;
    case 10: // gg
      // not a troll, jade
      while(*Peek)
        *(Poke++) = *(Peek++);
      break;
    case 11: // gc
      while(*Peek) {
        *Poke = *Peek;
        if(tolower(Peek[0]) == 'a') *Poke = '4';
        if(tolower(Peek[0]) == 'i') *Poke = '1';
        if(tolower(Peek[0]) == 'e') *Poke = '3';
        Poke++; Peek++;
        Replace(":)", ">:]", 0);
        Replace(";)", ">;]", 0);
      }
      UpperAll();
      break;
    case 12: // ca
      while(*Peek) {
        *Poke = *Peek;
        if(tolower(Peek[0]) == 'w') *(++Poke) = 'w';
        if(tolower(Peek[0]) == 'v') *(++Poke) = 'v';
        Replace("ing ","in ",0);
        Poke++;
        Peek++;
      }
      LowerAll();
      break;
    case 13: // ct
      strcpy(Poke, "D --> ");
      Poke += 6;
      while(*Peek) {
        *Poke = *Peek;
        Replace("strong","STRONG",0);
        Replace("loo","100",0);
        Replace("lue ","100 ",0);
        Replace("ool","001",0);
        if(tolower(Peek[0]) == 'x') *Poke = '%';
        Replace(" cross"," %",0);
        Replace(" doublecross"," %%",0);
        Poke++; Peek++;
      }
      break;
    case 14: // cg
      while(*Peek)
        *(Poke++) = toupper(*(Peek++));
      break;
    case 15: // cc
      while(*Peek) {
        *Poke = *Peek;
        Replace("h", ")(", 0);
        Replace("H", ")(", 0);
        Replace(":", "38", 0);
        if(*Peek == 'E') {
          *(Poke++) = '-';
          *Poke = 'E';
        }
        Poke++; Peek++;
      }
      break;
  }
  *Poke = 0;

  if(Troll[2] != 0) {
    char Buffer[768];
    TrollFilter(Buffer, Output, Troll+2);
    strcpy(Output, Buffer);
  }

  return Output;
}
// http://tibasicdev.wikidot.com/83smfont
void PrintX5Font(char *In, char *Config) {
  if(Config==NULL)
    Config = "@0001";

  char NewConfig[20];
  strcpy(NewConfig, Config);
  Config = NewConfig;

  int UseReverse = 0, SpecialFG = 0, SpecialBG = 0;
  if(!strcasecmp(Config, "r")) {
    Config = " ????";
    UseReverse = 1;
  }
  if(!strcasecmp(Config, "rainbow")) {
    SpecialFG = 1;
    SpecialBG = 1;
    Config = "@0001";
  }
  if(strlen(Config) >= 5) {
    if(toupper(Config[1])=='R' && toupper(Config[2])=='R') SpecialFG = 1;
    if(toupper(Config[3])=='R' && toupper(Config[4])=='R') SpecialBG = 1;
  }

  if(NeedSpaceBetweenX5Font)
    xchat_commandf(ph, "say  ");
  unsigned const char Font[96][6] = {
    { 0,  0,  0,  0,  0, 1}, /*   */ { 1,  1,  1,  0,  1, 1}, /* ! */ { 5,  5,  5,  0,  0, 3}, /* " */ {10, 31, 10, 31, 10, 5}, /* # */ 
    {14, 20, 14,  5, 30, 5}, /* $ */ { 5,  1,  2,  4,  5, 3}, /* % */ { 4, 10,  4, 10,  5, 4}, /* & */ { 1,  1,  1,  0,  0, 1}, /* ' */ 
    { 1,  2,  2,  2,  1, 2}, /* ( */ { 2,  1,  1,  1,  2, 2}, /* ) */ { 4, 21, 14, 21,  4, 5}, /* * */ { 0,  2,  7,  2,  0, 3}, /* + */ 
    { 0,  0,  1,  1,  2, 2}, /* , */ { 0,  0,  7,  0,  0, 3}, /* - */ { 0,  0,  0,  0,  1, 1}, /* . */ { 1,  1,  2,  4,  4, 3}, /* / */ 
    { 2,  5,  5,  5,  2, 3}, /* 0 */ { 2,  6,  2,  2,  7, 3}, /* 1 */ { 6,  1,  2,  4,  7, 3}, /* 2 */ { 6,  1,  2,  1,  6, 3}, /* 3 */ 
    { 4,  5,  7,  1,  1, 3}, /* 4 */ { 7,  4,  6,  1,  6, 3}, /* 5 */ { 3,  4,  7,  5,  7, 3}, /* 6 */ { 7,  1,  2,  4,  4, 3}, /* 7 */ 
    { 7,  5,  7,  5,  7, 3}, /* 8 */ { 7,  5,  7,  1,  6, 3}, /* 9 */ { 0,  1,  0,  1,  0, 1}, /* : */ { 0,  1,  0,  1,  2, 2}, /* ; */ 
    { 1,  2,  4,  2,  1, 3}, /* < */ { 0,  7,  0,  7,  0, 3}, /* = */ { 4,  2,  1,  2,  4, 3}, /* > */ { 6,  1,  2,  0,  2, 3}, /* ? */ 
    {14,  1, 13, 21, 14, 5}, /* @ */ { 2,  5,  7,  5,  5, 3}, /* A */ { 6,  5,  6,  5,  6, 3}, /* B */ { 3,  4,  4,  4,  3, 3}, /* C */ 
    { 6,  5,  5,  5,  6, 3}, /* D */ { 7,  4,  6,  4,  7, 3}, /* E */ { 7,  4,  6,  4,  4, 3}, /* F */ { 3,  4,  5,  5,  3, 3}, /* G */ 
    { 5,  5,  7,  5,  5, 3}, /* H */ { 7,  2,  2,  2,  7, 3}, /* I */ { 1,  1,  1,  5,  7, 3}, /* J */ { 5,  5,  6,  5,  5, 3}, /* K */ 
    { 4,  4,  4,  4,  7, 3}, /* L */ { 5,  7,  7,  5,  5, 3}, /* M */ { 6,  5,  5,  5,  5, 3}, /* N */ { 7,  5,  5,  5,  7, 3}, /* O */ 
    { 6,  5,  6,  4,  4, 3}, /* P */ { 7,  5,  5,  7,  3, 3}, /* Q */ { 6,  5,  6,  5,  5, 3}, /* R */ { 3,  4,  2,  1,  6, 3}, /* S */ 
    { 7,  2,  2,  2,  2, 3}, /* T */ { 5,  5,  5,  5,  7, 3}, /* U */ { 5,  5,  5,  2,  2, 3}, /* V */ { 5,  5,  5,  7,  5, 3}, /* W */ 
    { 5,  5,  2,  5,  5, 3}, /* X */ { 5,  5,  2,  2,  2, 3}, /* Y */ { 7,  1,  2,  4,  7, 3}, /* Z */ { 3,  2,  2,  2,  3, 2}, /* [ */ 
    { 4,  4,  2,  1,  1, 3}, /* \ */ { 3,  1,  1,  1,  3, 2}, /* ] */ { 2,  5,  0,  0,  0, 3}, /* ^ */ { 0,  0,  0,  0,  7, 3}, /* _ */ 
    { 2,  1,  0,  0,  0, 2}, /* ` */ { 0,  3,  5,  5,  3, 3}, /* a */ { 4,  6,  5,  5,  6, 3}, /* b */ { 0,  3,  4,  4,  3, 3}, /* c */ 
    { 1,  3,  5,  5,  3, 3}, /* d */ { 0,  2,  5,  6,  3, 3}, /* e */ { 1,  2,  3,  2,  2, 2}, /* f */ { 3,  5,  3,  1,  6, 3}, /* g */ 
    { 4,  6,  5,  5,  5, 3}, /* h */ { 1,  0,  1,  1,  1, 1}, /* i */ { 1,  0,  1,  5,  2, 3}, /* j */ { 4,  4,  5,  6,  5, 3}, /* k */ 
    { 3,  1,  1,  1,  1, 2}, /* l */ { 0, 26, 21, 21, 17, 5}, /* m */ { 0,  6,  5,  5,  5, 3}, /* n */ { 0,  2,  5,  5,  2, 3}, /* o */ 
    { 0,  6,  5,  6,  4, 3}, /* p */ { 0,  3,  5,  3,  1, 3}, /* q */ { 0,  5,  6,  4,  4, 3}, /* r */ { 0,  3,  2,  1,  3, 2}, /* s */ 
    { 2,  3,  2,  2,  1, 2}, /* t */ { 0,  5,  5,  5,  7, 3}, /* u */ { 0,  5,  5,  2,  2, 3}, /* v */ { 0, 17, 21, 21, 10, 5}, /* w */ 
    { 0,  5,  2,  2,  5, 3}, /* x */ { 0,  5,  5,  2,  4, 3}, /* y */ { 0, 15,  2,  4, 15, 4}, /* z */ { 3,  2,  4,  2,  3, 3}, /* { */ 
    { 1,  1,  1,  1,  1, 1}, /* | */ { 6,  2,  1,  2,  6, 3}, /* } */ { 0,  5, 10,  0,  0, 4}, /* ~ */
  };
  int Row; char *Peek;

  for(Row = 0; Row<5; Row++) {
    char RowBuffer[512];
    *RowBuffer = 0;
    char *Poke = RowBuffer;

    for(Peek = In; *Peek; Peek++) {
      int Char = *Peek;
      if(Char > '~' || Char < ' ') Char = '?';
      char FontBits = Font[Char-0x20][Row];
      char Column; char Width = Font[Char-0x20][5];
      for(Column=0;Column<Width+1;Column++)
        *(Poke++) = ((FontBits>>(Width-Column))&1)?'@':' ';
    }
    *(Poke++) = ' ';
    *Poke=0;

    char ColorBuffer[512];
    strcpy(ColorBuffer, "");
    Poke = ColorBuffer;

    if(!UseReverse) {
      *(Poke++) = 0x03;      *(Poke++) = Config[3];
      *(Poke++) = Config[4]; *(Poke++) = ',';
      *(Poke++) = Config[3]; *(Poke++) = Config[4];
    }

    for(Peek = RowBuffer; *Peek; Peek++) {
      if(SpecialFG == 1) {
        int x = rand2(5);
        NewConfig[1] = Rainbow[x][0];
        NewConfig[2] = Rainbow[x][1];
      }
      if(SpecialBG == 1) {
        int x = rand2(5);
        NewConfig[2] = Rainbow[x][0];
        NewConfig[3] = Rainbow[x][1];
      }

      if(!UseReverse&&(Peek==RowBuffer || Peek[-1]!=Peek[0] || SpecialFG==1)) {
        if(*Peek == '@') {
          *(Poke++) = 0x03;      *(Poke++) = Config[1];
          *(Poke++) = Config[2]; *(Poke++) = ',';
          *(Poke++) = Config[1]; *(Poke++) = Config[2];
        } else {
          *(Poke++) = 0x03;      *(Poke++) = Config[3];
          *(Poke++) = Config[4]; *(Poke++) = ',';
          *(Poke++) = Config[3]; *(Poke++) = Config[4];
        }
      }
      if(UseReverse && Peek!=RowBuffer && Peek[-1]!=Peek[0]) {
        *(Poke++) = 0x16;
      }
        
      *(Poke++) = Config[0];
    }

    *Poke=0;
    xchat_commandf(ph, "say %s", ColorBuffer);
  }
  NeedSpaceBetweenX5Font=1;
}

void Print35Font(char *In, char *Config) {
  if(Config==NULL)
    Config = "@0001";
  int i;
  unsigned const short Font[256] = {
    ['0']=075557, ['1']=022222, ['2']=074216, ['3']=071717,
    ['4']=011755, ['5']=061747, ['6']=075747, ['7']=044217,
    ['8']=075757, ['9']=071757, ['!']=020222, ['?']=020317,
    ['A']=055752, ['B']=065656, ['C']=034443, ['D']=065556,
    ['E']=074747, ['F']=044747, ['G']=035543, ['H']=055755,
    ['I']=072227, ['J']=075111, ['K']=055655, ['L']=074444,
    ['M']=055775, ['N']=055556, ['O']=075557, ['P']=044757,
    ['Q']=036552, ['R']=055656, ['S']=071747, ['T']=022227,
    ['U']=075555, ['V']=025555, ['W']=057555, ['X']=055255,
    ['Y']=022755, ['Z']=074217, [' ']=000000,
    ['^']=000052, ['-']=000700, ['+']=002720, ['=']=007070,
    ['.']=022000, [',']=062000, ['_']=070000, ['/']=042221,
    [':']=002020, [';']=062020, ['<']=012421, ['>']=042124,
    ['\\']=012224, ['\"']=000055, ['\'']=000022, ['\2']=077777,
  };
  if(NeedSpaceBetweenX5Font)
    xchat_commandf(ph, "say  ");
  int Row; char *Peek;
  int ColorType[2]={0,0};
  for(i=0;i<2;i++) {
    if(toupper(Config[(i<<1)+1])=='R' && toupper(Config[(i<<1)+2])=='B') ColorType[i] = 1; // horiz rainbow
    if(toupper(Config[(i<<1)+1])=='R' && toupper(Config[(i<<1)+2])=='H') ColorType[i] = 1; // horiz rainbow
    if(toupper(Config[(i<<1)+1])=='R' && toupper(Config[(i<<1)+2])=='V') ColorType[i] = 2; // vert rainbow
  }

  for(Row = 0; Row<5; Row++) {
    unsigned short ShiftBy = Row*3;
    unsigned short AndBy = 0x7<<ShiftBy;
    char RowBuffer[512];
    strcpy(RowBuffer, "");
    char *Poke = RowBuffer;

    for(Peek = In; *Peek; Peek++) {
      int Char = toupper(*Peek);
      char FontBits = (Font[Char]&AndBy)>>ShiftBy;
      char Column;
      for(Column=0;Column<3;Column++)
        *(Poke++) = ((FontBits<<=1)&8?'@':' ');
      *(Poke++) = ' ';
    }
    *Poke=0;

    char ColorBuffer[512];
    strcpy(ColorBuffer, "");
    Poke = ColorBuffer;

    int Index = 0;

    if(ColorType[1] != 0)
      memcpy(Config+(2)+1,Rainbow[Row%5],sizeof(char)*2);
      //memcpy(Config+(2)+1,"00",sizeof(char)*2);

    *(Poke++) = 0x03;      *(Poke++) = Config[3];
    *(Poke++) = Config[4]; *(Poke++) = ',';
    *(Poke++) = Config[3]; *(Poke++) = Config[4];
    *(Poke++) = Config[0];


    for(Peek = RowBuffer; *Peek; Peek++) {
       for(i=0;i<2;i++) {
        switch(ColorType[i]) {
          case 1:
            if(0==(Index&3))
              memcpy(Config+(i<<1)+1,Rainbow[(Index/4)%5],sizeof(char)*2);
            break;
          case 2:
            memcpy(Config+(i<<1)+1,Rainbow[Row%5],sizeof(char)*2);
            break;
        }
      }
      Index++;

      if(Peek==RowBuffer || Peek[-1]!=Peek[0]) {
        if(*Peek == '@') {
          *(Poke++) = 0x03;      *(Poke++) = Config[1];
          *(Poke++) = Config[2]; *(Poke++) = ',';
          *(Poke++) = Config[1]; *(Poke++) = Config[2];
        } else {
          *(Poke++) = 0x03;      *(Poke++) = Config[3];
          *(Poke++) = Config[4]; *(Poke++) = ',';
          *(Poke++) = Config[3]; *(Poke++) = Config[4];
        }
      }
      *(Poke++) = Config[0];
    }

    *Poke=0;
    xchat_commandf(ph, "say %s", ColorBuffer);
  }
  NeedSpaceBetweenX5Font=1;
}

char *AcidText(char *NewString, char *Input) {
  char *Poke = NewString, *Peek, *StartHere = Input;
  *(Poke++) = 2;

  int ColorFrequency = 7;
  if(strlen(Input) > 100) ColorFrequency = 15;
  if(strlen(Input) > 200) ColorFrequency = 31;
  if(strlen(Input) > 300) ColorFrequency = -1;

  for(Peek = StartHere;*Peek;Peek++) {
    if(Peek == StartHere || (isgraph2(*Peek) && (ColorFrequency!=-1) && ((rand()&ColorFrequency) == 0))) {
      int Color1 = rand()&15;
      while(Color1 == 1 || Color1 == 14 || Color1 == 0)
        Color1 = rand()&15;
      char ColorString[10];
      *(Poke++) = 3;
      sprintf(ColorString, "%.2i", Color1);
      *(Poke++) = ColorString[0];
      *(Poke++) = ColorString[1];
      if(Peek == StartHere) {
        *(Poke++) = ',';
        *(Poke++) = '0';
        *(Poke++) = '1';
      }
    }
    if((rand()&7) == 0)
      *(Poke++) = 0x1f;
    char k = *Peek;
    *(Poke++) = (rand()&1)?toupper(k):tolower(k);
  }
  *Poke = 0;
  return NewString;
}

// Used for /spark slashf
static char *sparkles_text_unescape(char *Poke, char *Peek) {
// Takes a string given, and interprets \n and friends
  char *End = Peek+strlen(Peek);
  while(*Peek != 0 && (Peek < End)) {
    // (Peek<End) so it doesn't crash when Peek accidently skips over the end
    *Poke++ = *Peek;
    if(*Peek == '\\') {
      Peek++;
      switch(*Peek) {
        case '\\': *(Poke-1) = '\\'; break; // Escaped '\'
        case 'R': *(Poke-1)  = 0x16; break; // Reversed
        case 'i': *(Poke-1)  = 0x1d; break; // Italic
        case '1': *(Poke-1)  = 0x01; break; // for making bots do actions
        case 'u': *(Poke-1)  = 0x1f; break; // Underline
        case 'b': *(Poke-1)  = 0x02; break; // Bold
        case 'c': *(Poke-1)  = 0x03; break; // Colored
        case 'p': *(Poke-1)  = 0x0f; break; // Plain
        case 'a': *(Poke-1)  = '\a'; break; // Bell
        case 'n': *(Poke-1)  = '\n'; break; // Newline
        case 't': *(Poke-1)  = '\t'; break; // Tab
        case 'r': *(Poke-1)  = '\r'; break; // Carriage return
        case 'x': *(Poke-1)  =  strtol(Peek+1, &Peek, 16); break; // Read as hex
        case 'h': *(Poke-1)  =  0x03; *(Poke++) = '0'; *(Poke++) = '1'; *(Poke++) = ','; *(Poke++) = '0'; *(Poke++) = '1'; break;
        case 'w': *(Poke-1)  =  0x03; *(Poke++) = '0'; *(Poke++) = '0'; *(Poke++) = ','; *(Poke++) = '0'; *(Poke++) = '0'; break;
        default: *(Poke-1)   =  '?'; break; // Unrecognized
      }
    }
    Peek++;
  }

  *Poke = 0;
  return Poke;
}

// Given a list of words to choose from, Adj() selects one at random
// Named because it was originally for randomly choosing adjectives
static char *Adj(int Amount, char *Word, ...) {
   va_list vl;
   va_start(vl,Word);
   char *Chk = Word;
   int Which = rand2(Amount);
   while(Which){
    if(Chk == NULL) break;
    Which--;
    Chk=va_arg(vl,char*);
   }
   va_end(vl);
   return(Chk);
   return(NULL); // no matches
}

// add to this list if you want, but be sure to update the number in SomeSpecies()
static char *RandomSpecies[] = {"fox","wolf","kangaroo","raccoon","deer","skunk","hamster","dingo","coyote","kitty","squirrel","bunny","rabbit","dragon","canine","feline","vulpine","pony","lemming","squirrel","otter","chakat","spacecat","sergal","porcupine","hedgehog","koala", NULL};
static char *SomeSpecies(){return(RandomSpecies[rand2(27)]);}

#if ENABLE_NSFW_CONTENT
// * QNinja eyetwitches at the.... accuracy
static void GenDavYiff(char *CmdBuf, const char *Victim) {
  strcpy(CmdBuf, "");
  if (Victim==NULL) return;
  if (!strcasecmp(Victim,"")) return;
  switch(rand()&7) {
    case 0: case 1: sprintf(CmdBuf, "running to %s", Victim); break;
    case 2: case 3: sprintf(CmdBuf, "jump on %s", Victim); break;
    case 4: case 5: sprintf(CmdBuf, "nestles %son %s", Adj(2, "", "tight "), Victim); break;
    case 6: case 7:
      sprintf(CmdBuf, "stops to %s %s", Adj(4,"hug","tickle","tickle","kiss"), Victim); break;
  }
  strcat(CmdBuf, " and");
  switch(rand()&7) {
    case 0: case 1: sprintf(CmdBuf, "%s hug %s", CmdBuf, Victim); break;
    case 2: case 3: sprintf(CmdBuf, "%s pets %s", CmdBuf, Victim); break;
    case 4: case 5: sprintf(CmdBuf, "%s kisses %s", CmdBuf, Victim); break;
    case 6: case 7: sprintf(CmdBuf, "%s tickles %s", CmdBuf, Victim); break;
  }
  strcat(CmdBuf, " then");
  switch(rand()&15) {
    case 0: case 1: sprintf(CmdBuf, "%s nibbles on the tits of %s", CmdBuf, Victim); break;
    case 2: sprintf(CmdBuf, "%s lick the tail hole of %s", CmdBuf, Victim); break;
    case 3: case 4: sprintf(CmdBuf, "%s give a big cookie to %s", CmdBuf, Victim); break;
    case 5: sprintf(CmdBuf, "%s give a big bouquet of red roses to %s", CmdBuf, Victim); break;
    case 6: sprintf(CmdBuf, "%s lick the muzzle of %s", CmdBuf, Victim); break;
    case 7: sprintf(CmdBuf, "%s give a red rose to %s", CmdBuf, Victim); break;
    case 8: sprintf(CmdBuf, "%s give a pizza to %s", CmdBuf, Victim); break;
    case 9: sprintf(CmdBuf, "%s goes on his knee and give a diamond ring to %s", CmdBuf, Victim); break;
    case 10: sprintf(CmdBuf, "%s kick the balls of %s to the moon", CmdBuf, Victim); break;
    case 11: sprintf(CmdBuf, "%s begins to crying", CmdBuf); break;
    case 12: sprintf(CmdBuf, "%s stroke the tits of %s", CmdBuf, Victim); break;
    case 13: sprintf(CmdBuf, "%s give a magic sword to %s", CmdBuf, Victim); break;
    case 14: sprintf(CmdBuf, "%s running away", CmdBuf); break;
    case 15: sprintf(CmdBuf, "%s sticks his finger in the cunt of %s", CmdBuf, Victim); break;
  }
}

static void DoDavYiff(char *Victim) {
  char Buffer[512]; int i;
  GenDavYiff(Buffer, Victim);
  switch(rand()&7) {
    case 0: case 1: break;
    case 2: case 3: strcat(Buffer, " ^ w ^");break;
    case 4: case 5: case 6: case 7:
      for(i=rand()%16;i;i--)
        strcat(Buffer, " <3");
      break;
  }
  for(i=0;Buffer[i];i++)
    Buffer[i]=tolower(Buffer[i]);
  xchat_commandf(ph, "me %s", Buffer);
}

static int yiff_cb(char *word[], char *word_eol[], void *userdata) {
  // checking if null arguments were passed
  if (word[2]==NULL) return(XCHAT_EAT_ALL);
  if (!strcasecmp(word[2],"")) return(XCHAT_EAT_ALL);

  // alter the number if you add/take away yiffscript choices
  int MaxYiffScript = 44;

  int action = -1;
  int BillyMaysMode = 0;

   // fetch these however your IRC client does it
//   const char *MyNick = xchat_get_info(ph,"nick");
  const char *Victim = word[2];

  char CommandBuf[2048];  // I'm sick and tired of buffer overflow crashing my IRC bot,
  char ChainBuffer[2048]; // so I'll just raise the buffers to stupid sizes.
  sprintf(CommandBuf, "fails to find any YiffScript actions that match id %i", action);
  strcpy(ChainBuffer, "");

  char *MySpecies=SomeSpecies();
  char *YourSpecies=SomeSpecies();
  char *ReplyWith = "me";

  int YiffChain = 1;
  int ActionSet = 0;
  char *Emoticon = Adj(8,"","","^w^","^ w ^","<3",":3","=^____^=","=^________^=");
  int Rainbows = 0; 
  char *Conjoin = ", and then ";

  // scan through these however your IRC client does it
  int i;
  int ShuffleLevel = 0;

  char *HSTroll = NULL;
  for(i=3;word[i]!=NULL && strcasecmp(word[i],"");i++) {
      if(!strcasecmp(word[i],"-am")) // set action
        action  = strtol(word[i+1],NULL,10);
      if(!strcasecmp(word[i],"-ms")) // set my species
        MySpecies = word[i+1];
      if(!strcasecmp(word[i],"-ys")) // set your species
        YourSpecies=word[i+1];
      if(!strcasecmp(word[i],"-hstroll"))
        HSTroll=word[i+1];
      if(!strcasecmp(word[i],"-bm") || !strcasecmp(word[i],"-allcaps"))
        BillyMaysMode = 1;
      if(!strcasecmp(word[i],"-altcaps"))
        BillyMaysMode = 2;
      if(!strcasecmp(word[i],"-randcaps"))
        BillyMaysMode = 3;
      if(!strcasecmp(word[i],"-snuggle")) {
        ActionSet = 1; MaxYiffScript = 10;}
      if(!strcasecmp(word[i],"-davyiff")) {
        ActionSet = 2;}
      if(!strcasecmp(word[i],"-fuk")) {
        ActionSet = 3; MaxYiffScript = 40;
      }
      if(!strcasecmp(word[i],"-list")) {
        ActionSet = 4;
        // no exploits plz
        if(strlen(word[i+1]) >= 30) return XCHAT_EAT_ALL;
        if(strchr(word[i+1], '.'))  return XCHAT_EAT_ALL;
        if(strchr(word[i+1], '/'))  return XCHAT_EAT_ALL;
        if(strchr(word[i+1], '\\')) return XCHAT_EAT_ALL;
        if(strcasecmp(word[i+1], CustomYiffName)) {
          strcpy(CustomYiffName, word[i+1]);
          if(CustomYiffBuffer) free(CustomYiffBuffer);
          char Buffer[260];
          sprintf(Buffer, "%s/Sparkles/yiff%s.txt", xchat_get_info(ph, "xchatdirfs"), word[i+1]);
          CustomYiffBuffer = ReadTextFile(Buffer);
          if(!CustomYiffBuffer) {
            xchat_printf(ph, "Could not open %s\n", Buffer);
            return XCHAT_EAT_ALL;
          }
          CustomYiffPointers[0] = CustomYiffBuffer;
          CustomYiffCount = 1;
          char *Poke = CustomYiffBuffer;
          while(1) {
            Poke = strchr(Poke+1, '\n');
            if(!Poke) break;
            if(Poke[-1] == '\r') Poke[-1] = 0;
            *Poke = 0;
            CustomYiffPointers[CustomYiffCount++] = Poke+1;
          }
        }
        MaxYiffScript = CustomYiffCount;
      }
      if(!strcasecmp(word[i],"-rainbow"))
        Rainbows=1;
      if(!strcasecmp(word[i],"-emote") || !strcasecmp(word[i],"-suffix"))
        Emoticon = word[i+1];
      if(!strcasecmp(word[i],"-conjoin"))
        Conjoin = word[i+1];
      if(!strcasecmp(word[i],"-vi"))
        Victim = word[i+1];
      if(!strcasecmp(word[i],"-bold"))
        strcat(ChainBuffer, "\2");
      if(!strcasecmp(word[i],"-italic"))
        strcat(ChainBuffer, "\x1d");
      if(!strcasecmp(word[i],"-underline"))
        strcat(ChainBuffer, "\x1f");
      if(!strcasecmp(word[i],"-inverse"))
        strcat(ChainBuffer, "\x16");
      if(!strcasecmp(word[i],"-prefix"))
        strcat(ChainBuffer, word[i+1]);
      if(!strcasecmp(word[i],"-say")) // use /say instead (so you can use cmdstack)
        ReplyWith = "say";
      if(!strcasecmp(word[i],"-shuffle"))
        ShuffleLevel=strtol(word[i+1],NULL,10);
      if(!strcasecmp(word[i],"-yc")) { // start a yiffchain!
        YiffChain=strtol(word[i+1],NULL,10)&3;
        // going too high crashes stuff with all the yiffiness so I'll cap it at 3
      }
   }
  if(action == -1) {
    do {
      action = rand2(MaxYiffScript);
    } while(action == LastYiff);
  }
  if(YiffChain != 1) Rainbows = 0;
  const char *ReplaceWith[] = {xchat_get_info(ph, "nick"), Victim, MySpecies, YourSpecies, xchat_get_info(ph, "channel")};

  if(YiffChain == 0) sprintf(ChainBuffer, "throws a pizza party and invites %s", Victim);
  for(;YiffChain;YiffChain--) { // really sloppy way of implementing yiffchains
    if(ActionSet == 0) {
      if (action ==  0) sprintf(CommandBuf,"rubs %s's %s while groping at his crotch with his feet",Victim,Adj(3,"member","footpaws","knot"));
      if (action ==  1) sprintf(CommandBuf,"grabs %s, twirls him around like a dancer and places a kiss on his lips, fondling his tailhole for a second so he can taste it later ",Victim);
      if (action ==  2) sprintf(CommandBuf,"rubs %s's hard %s while reaching back for some extra tailhole fun",Victim,Adj(2,"knot","member"));
      if (action ==  3) sprintf(CommandBuf,"snuggletackles %s and starts romping on top of %s",Victim, Adj(2, "him","her"));
      if (action ==  4) sprintf(CommandBuf,"takes %s's %s %s into his maw and begins suckling like one would their mother's breast", Victim,Adj(8,"stiff","hard","long","tasty","thick","erect","throbbing","pulsing"),Adj(3,"rod","shaft","member"));
      if (action ==  5) sprintf(CommandBuf,"puts his fingers on %s's mouth to shush him as he starts working his %s %s%s",Victim,Adj(2,"throbbing","pulsing"),YourSpecies,Adj(4,"hood"," member"," penis"," rod"));
      if (action ==  6) sprintf(CommandBuf,"caresses %s's furry ballsac, inhaling %s's sweet musk through his nostrils, his %s starting to grow from all the sensations", Victim, Victim, Adj(2,"member","rod"));
      if (action ==  7) sprintf(CommandBuf,"murrs, humping softly against %s's hand, twisting the tip of his finger in %s's %s", Victim, Victim, Adj(3,"butthole","tailhole","tailstar"));
      if (action ==  8) sprintf(CommandBuf,"licks along %s's silky lips, taking %s's exposed %s%s and plays with the tip, rubbing the underside with his thumb slowly towards the tip", Victim, Victim, YourSpecies, Adj(3,"hood","hood"," member"));
      if (action ==  9) sprintf(CommandBuf,"gets a firm grip on %s's hip as he thrusts his %shood deep within his tight and inexperienced %s, unexpectedly ramming his knot causing an orgasm of pain and pleasure for %s",Victim,MySpecies,Adj(2,"tailhole","tailstar"),Victim);
      if (action == 10){sprintf(CommandBuf,"emerges from his sheathe, his hard knot glistening in the moist air"); YiffChain++; }
      if (action == 11) sprintf(CommandBuf,"pokes a finger into %s's %s sheath, tasting the tip of his finger in excitement", Victim,Adj(2,"moist","wet"));
      if (action == 12) sprintf(CommandBuf,"%s %s %s all over %s",Adj(4,"sprays","blasts","jets","squirts"),Adj(8,"hot","sticky","slick","warm","delicious","yummy","tasty","illegal"),Adj(4,"sperm","seed","cum","semen"),Victim);
      if (action == 13) sprintf(CommandBuf,"clenches his firm, supple buttocks around the %s head of %s's %s%s", Adj(4,"thick","hard","slick","wet"), Victim, YourSpecies, Adj(4,"cock","hood"," member", "dick"));
      if (action == 14) sprintf(CommandBuf,"runs his %s along %s's soft and slender thighs, running up to his %s %s%s ",Adj(2,"fingers","paws"), Victim, Adj(3,"thick and juicy","partially erect","hard, throbbing"), YourSpecies, Adj(4,"dick", "hood","cock"," member"));
      if (action == 15) sprintf(CommandBuf,"wraps his hands around %s's back, moving downward while kissing along his soft yet sturdy chest and abs",Victim);
      if (action == 16) sprintf(CommandBuf,"runs his %s through %s's silky fur, rubbing his %s to get him excited",Adj(2,"fingers","paws"),Victim,Adj(2,"dick","ear"));
      if (action == 17) sprintf(CommandBuf,"slips his tongue into %s's moist sheath, taking in all the flavors of his %s musk",Victim,YourSpecies);
      if (action == 18) sprintf(CommandBuf,"takes %s's entire %s length into his mouth, leaking precum in excitement",Victim, YourSpecies);
      if (action == 19) sprintf(CommandBuf,"cuddles close to %s, his excitement poking %s from behind",Victim,Victim);
      if (action == 20) sprintf(CommandBuf,"pushes down onto %s's lap with his weight, thrusting %s's knot deep within his %s",Victim,Victim, Adj(2,"tailstar","tailhole"));
      if (action == 21) sprintf(CommandBuf,"rubs his petite bottom on %s's well-endowed %s-cock",word[2],YourSpecies);
      if (action == 22) sprintf(CommandBuf,"murrs as he slides his paw over %s's %s %s %s",Victim, Adj(9,"throbbing","huge","gigantic","enormous","freaking huge", "ridiculously sized", "illegally sized","massive","itty bitty"), YourSpecies,Adj(4,"cock","penis","length","member"));
      if (action == 23) sprintf(CommandBuf,"pushes down on %s's rock hard %s%s with his palm, sliding around with his precum", Victim, YourSpecies,Adj(4,"cock","hood"," member"," rod"));
      if (action == 24) sprintf(CommandBuf,"takes %s's entire %s length into his mouth, gagging as he swallows the knot", Victim, YourSpecies);
      if (action == 25) sprintf(CommandBuf,"slides %s fingers into %s's %s %s while fondling hir sheathe",Adj(4,"two","three","four","over 9000"),Victim,Adj(4,"juicy","tight","erect","wet"),Adj(3,"pussy","vagina","cunt"));
      if (action == 26) sprintf(CommandBuf,"tries to go down on %s's sweet %s-gina, but is pushed back by hir throbbing %s",Victim,YourSpecies,Adj(2,"member","rod"));
      if (action == 27) sprintf(CommandBuf,"takes a painfully hard grip on %s's %s%s from behind and %s his knot right up %s %s %s", Victim, YourSpecies, Adj(3,"hood","cock"," member"), Adj(5,"jams","forces","shoves","pushes","slams"), Adj(3,"his","her","hir"), Adj(2,"virgin","tight"), Adj(2,"tailhole","tailstar"));
      if (action == 28) sprintf(CommandBuf,"slides his throbbing %s%s along %s's luscious %s breasts, leaving a trail of precum along %s sweat-glistened chest", MySpecies, Adj(3,"hood","cock"," member"), Victim, YourSpecies, Adj(3,"her","his","hir"));
      if (action == 29) sprintf(CommandBuf,"puts %s muzzle between %s's smooth thighs and starts lapping at %s %s %sclit", Adj(3,"her","his","hir"), Victim, Adj(2,"her","hir"), Adj(3,"slippery","sensitive","erect"), YourSpecies);
      if (action == 30) sprintf(CommandBuf,"gently slides his paw over %s's right foot while gliding a finger over his tailhole in anticipation",Victim);
      if (action == 31){sprintf(CommandBuf,"drags a foxy claw across the ridges of %s's circumcision scar",Victim); YiffChain++; }
      if (action == 32) sprintf(CommandBuf,"%s a huge creamy yiffload all over %s", Adj(4,"busts","blasts","shoots","jets"), Victim);
      if (action == 33) sprintf(CommandBuf,"takes %s and ravages them from behind with a huge %s boner.",word[2],MySpecies);
      if (action == 34) sprintf(CommandBuf,"aims and shoots, but misses %s's mouth, leaving a mess of jizz dripping down %s's chin!",Victim,Victim);
      if (action == 35) sprintf(CommandBuf,"%s gallons of %s jizz all over %s", Adj(2,"blasts","pumps"), MySpecies, word[2]);
      if (action == 36) sprintf(CommandBuf,"howls with pleasure as %s pulls his knot out of his hole, stretching it to an obscene size before a loud \"POP\" is heard and %s's %sjuice cascades down his already drenched thighs",word[2],Victim,YourSpecies);
      if (action == 37) sprintf(CommandBuf,"nibbles softly on the carrot protruding from %s's juicy hole as %s's fluffy bunnytail tickles %s face",Victim,Victim, Adj(3,"his","her", "hir"));
      if (action == 39) sprintf(CommandBuf,"tailwavies~ at %s, showing him his engorged, dripping %s%s",Victim, MySpecies, Adj(3,"hood","cock"," member"));
      if (action == 40) sprintf(CommandBuf,"critches the inside of %s's rosebud with his entire hand while lapping up the fountain of %s butter escaping from %s's engorged member",Victim,YourSpecies,Victim);
      if (action == 41) sprintf(CommandBuf,"rubs his paws through %s's %sfuzz, sticking his muzzle under the %s's tail giving %s %s a nice warm lick", Victim, Adj(3,"butt","butt","ass"), YourSpecies, Adj(2,"his","her"), Adj(2,"tailstar","tailhole"));
      if (action == 42) sprintf(CommandBuf,"goes apeshit and jumps on %s with a pulsing hard cock",Victim);
      if (action == 43) sprintf(CommandBuf,"grabs %s and kisses %s on the lips deeply, slowly starting to make out with the %s", Victim, Adj(2,"him","her"), YourSpecies);
    }
    if(ActionSet == 1) {
      if (action ==  0) sprintf(CommandBuf,"looks at %s, charges to them, jumps and snuggles!", Victim);
      if (action ==  1) sprintf(CommandBuf,"appears behind %s and wraps his arms around %s's midsection playfully!", Victim,Victim);
      if (action ==  2) sprintf(CommandBuf,"glomps %s, nuzzling him!", Victim);
      if (action ==  3) sprintf(CommandBuf,"hugs %s tightly, enjoying his warmth", Victim);
      if (action ==  4) sprintf(CommandBuf,"tackles %s to the ground in a hug, nuzzling him~", Victim);
      if (action ==  5) sprintf(CommandBuf,"snuggles with %s, murring~", Victim);
      if (action ==  6) sprintf(CommandBuf,"hugs %s, giving them a small kiss on the cheek", Victim);
      if (action ==  7) sprintf(CommandBuf,"tacklesnugs %s!!", Victim);
      if (action ==  8) sprintf(CommandBuf,"nuzzles %s, murring and smiling~", Victim);
      if (action ==  9) sprintf(CommandBuf,"playfully tickles %s, huggling them afterwards~", Victim);
    }
    if(ActionSet == 2) {
      GenDavYiff(CommandBuf, Victim);
    }
    if(ActionSet == 3) {
      if (action ==  0) sprintf(CommandBuf,"toch %s inapritly", Victim);
      if (action ==  1) sprintf(CommandBuf,"lok at %s but!", Victim);
      if (action ==  2) sprintf(CommandBuf,"chukl @ %s", Victim);
      if (action ==  3) sprintf(CommandBuf,"sap %s on the but", Victim);
      if (action ==  4) sprintf(CommandBuf,"get nakd 4 %s 2 wach", Victim);
      if (action ==  5) sprintf(CommandBuf,"laf @ %s's smal dik! lol!", Victim);
      if (action ==  6) sprintf(CommandBuf,"suk %s dik! omg big!", Victim);
      if (action ==  7) sprintf(CommandBuf,"lik her bobs & let %s watch!!!!!!", Victim);
      if (action ==  8) sprintf(CommandBuf,"msg her pus & com al ovr %s! sary!!!", Victim);
      if (action ==  9) sprintf(CommandBuf,"kis %s width pashin", Victim);
      if (action == 10) sprintf(CommandBuf,"rub %s dik til it skwart! yam!", Victim);
      if (action == 11) sprintf(CommandBuf,"cal %s a jek!", Victim);
      if (action == 12) sprintf(CommandBuf,"finghersef ten shuv figner up %s but!", Victim);
      if (action == 13) sprintf(CommandBuf,"frat on %s dik", Victim);
      if (action == 14) sprintf(CommandBuf,"jugl %s bals", Victim);
      if (action == 15) sprintf(CommandBuf,"chap %s hed of", Victim);
      if (action == 16) sprintf(CommandBuf,"cal %s a uqly nager! stop swimin!", Victim);
      if (action == 17) sprintf(CommandBuf,"pok %s jus 4 fun :D!", Victim);
      if (action == 18) sprintf(CommandBuf,"tak cloths off & let %s tak pics", Victim);
      if (action == 19) sprintf(CommandBuf,"get nakd & clim in2 %s's bed & suck %s dik undie teh covars", Victim, Victim);
      if (action == 20) sprintf(CommandBuf,"pop in %s tolet. it stink new!", Victim);
      if (action == 21) sprintf(CommandBuf,"tint %s by slap hre but", Victim);
      if (action == 22) sprintf(CommandBuf,"let %s stik his dik in witey puss. omg ur so bag!", Victim);
      if (action == 23) sprintf(CommandBuf,"por iskreem don %s pant", Victim);
      if (action == 24) sprintf(CommandBuf,"sit on %s dik & jigel her bobs", Victim);
      if (action == 25) sprintf(CommandBuf,"pop out of %s clost & shov %s dik up her but SUPRIS BUTSEKS", Victim, Victim);
      if (action == 26) sprintf(CommandBuf,"takl & hug %s", Victim);
      if (action == 27) sprintf(CommandBuf,"stik dik in %s boootie! wowh dat tite!", Victim);
      if (action == 28) sprintf(CommandBuf,"ripe %s in shawer", Victim);
      if (action == 29) sprintf(CommandBuf,"smak %s tity arond&rond", Victim);
      if (action == 30) sprintf(CommandBuf,"pul %s nepal tal melk cum out then i dranc melk! yum!", Victim);
      if (action == 31) sprintf(CommandBuf,"rip of %s close so %s nood wow %s youre prity..prity UGLY lol!!!!!", Victim, Victim, Victim);
      if (action == 32) sprintf(CommandBuf,"com in %s then pul out & lik joos. wowh test lek apl!", Victim);
      if (action == 33) sprintf(CommandBuf,"salp %s in fayc wit dik & P in aye, wops!", Victim);
      if (action == 34) sprintf(CommandBuf,"skwart don %s throught. r u stil thurst?", Victim);
      if (action == 35) sprintf(CommandBuf,"sho of dik 2 %s! dont laff @ me u jek!!!", Victim);
      if (action == 36) sprintf(CommandBuf,"dra pic of hishelf on %s bely wit prekom", Victim);
      if (action == 37) sprintf(CommandBuf,"dres dik up in tophat & shuv it up %s puss", Victim);
      if (action == 38) sprintf(CommandBuf,"rip of %s nepals & glu on hes bals", Victim);
      if (action == 39) sprintf(CommandBuf,"stik dik in %s ear & P! hehahe u got swimer year!", Victim);
    }
    if(ActionSet == 4) {
      *CommandBuf = 0; // probably don't need this
      TextInterpolate(CommandBuf, CustomYiffPointers[action], '%', "mysSc", ReplaceWith);
    }
    if(NULL != strstr(CommandBuf,"fails to")) {
      YiffChain++;
      action = rand2(MaxYiffScript); // pick a new yiff action
      continue;
    }
    else {
      strcat(ChainBuffer, CommandBuf);
      if(YiffChain != 1) strcat(ChainBuffer, Conjoin); // "and after that"
    }
/*
<NovaYoshi> Note the "and then"
<Kyurel> "and then"
<Kyurel> is that the best you can come up with?
<NovaYoshi> yes
<Kyurel> you should have a whole bunch of conjoining strings
<Kyurel> so that it's not like
<Kyurel> "and then... and then... and then... and then"
<Kyurel> maybe
<Kyurel> "next... then... thenceforth... finally..."
*/
     action = rand2(MaxYiffScript); // pick a new yiff action
  }
   sprintf(CommandBuf, "%s %s", ChainBuffer, Emoticon); // append a smiley

   if(BillyMaysMode==1 || !strcasecmp(Victim,"Billy Mays") || !strcasecmp(Victim,"BillyMays")) {
     for(i=0;CommandBuf[i];i++)
       CommandBuf[i]=toupper(CommandBuf[i]);
   }
   if(BillyMaysMode==2) {
     for(i=0;CommandBuf[i];i++)
       if(i&1)
         CommandBuf[i]=toupper(CommandBuf[i]);
       else
         CommandBuf[i]=tolower(CommandBuf[i]);         
   }
   if(BillyMaysMode==3) {
     for(i=0;CommandBuf[i];i++)
       if(rand()&1)
         CommandBuf[i]=toupper(CommandBuf[i]);
       else
         CommandBuf[i]=tolower(CommandBuf[i]);         
   }
   if(Rainbows) {
     int j;
     char *Poke = ChainBuffer;
     for(i=0,j=0;CommandBuf[i];i++) {
       if(isgraph2(CommandBuf[i]) && !(i&3)) {
         *(Poke++) = 0x03;
         *(Poke++) = Rainbow[j][0];
         *(Poke++) = Rainbow[j++][1];
         if(j == 5) j = 0;
       }
       *(Poke++) = CommandBuf[i];
     }
     *Poke = 0;
     strcpy(CommandBuf,ChainBuffer);
   }

   LastYiff=action;
 
   if(ShuffleLevel > 0) {
     ShuffleChars(ChainBuffer, CommandBuf,  ShuffleLevel,0);
     strcpy(CommandBuf, ChainBuffer);
   }

   if(HSTroll == NULL)
     xchat_commandf(ph, "%s %s", ReplyWith, CommandBuf);
   else {
     xchat_commandf(ph, "me %s", TrollFilter(ChainBuffer, CommandBuf, HSTroll));
   }
   return XCHAT_EAT_ALL;   /* eat this command so xchat and other plugins can't process it */
}
#endif

static int Activity2Focus_cb(char *word[], void *userdata) {
  NeedSpaceBetweenX5Font = 0;
  if(Activity2Focus)
    xchat_command(ph,"gui focus");
  return XCHAT_EAT_NONE;
}

static int RawServer_cb(char *word[], char *word_eol[], void *userdata) {
  // ERROR :Closing Link: Nick[host] services.anthrochat.net (NickServ (GHOST command used by Nick2))
  if(!DisableAutoGhostChainBreak && !strcasecmp(word[1], "ERROR") && NULL != strstr(word[3], "GHOST"))
    GhostDelayTime = (unsigned)time(NULL);

  if(GrabbingTopic && !strcasecmp(word[2],"332")) {
    char *Copy = word_eol[5];
    if(*Copy == ':')
      Copy++;
    xchat_commandf(ph, "settext /topic %s", Copy);
    xchat_commandf(ph, "setcursor 7");
    GrabbingTopic = 0;
    return XCHAT_EAT_ALL;
  }

  if(DisablePlusJFix || !RejoinKick)
    return XCHAT_EAT_NONE;
  if(!strcasecmp(word[2],"495")) { // "can't join, because +J"
    char *Channel = word[4];    
    if(NULL!=strstr(word_eol[6], "after being kicked to rejoin")) {
      char *N = strstr(word_eol[6], "must wait ");
      if(N != NULL) {
        int Seconds = strtol(N+10,NULL,10);
        xchat_commandf(ph,"spark spawnquiet %i.4 s join %s", Seconds, Channel);
      }
    }
  }
  return XCHAT_EAT_NONE;
}

static const char *SafeGet(const char *Orig, char *Substitute) {
// For functions that freak out when given NULL instead of a string
  if(Orig != NULL) return(Orig);
  if(Substitute != NULL) return(Substitute);
  return("(null)");
}

static char *Backwords(const char *Inp, char *Out) {
  int i, temp, Len; char *Seek = Out, *Fix;

  for(i=strlen(Inp)-1;i!=-1;i--)
    *(Seek++) = Inp[i];
  *Seek = 0;

  for(Seek = Out;;Seek++) {
    if((Seek!=Out) && ((*Seek == ' '||!*Seek) && Seek[-1]!=' ')) {
      for(Fix = Seek;Fix != Out && Fix[-1] != ' ';Fix--);
      if(NULL!=strchr(Fix, ' ')) Len = strchr(Fix, ' ') - Fix;
      else Len = strlen(Fix);
      for(i=0;i<=(Len>>1)-1;i++) {
         temp = Fix[i];
         Fix[i] = Fix[Len-1-i];
         Fix[Len-1-i] = temp;
      }
    }
    if(!*Seek)
      break;
  }
  return Out;
}

static int WhatNetwork_cb(char *word[], void *userdata) {
// Just in case filenames get damaged, I can still see from my logs what network they're from
  if(DisableShowNetworkOnJoin) return XCHAT_EAT_NONE;
  if(ForceUTF8) {
    if(strcasecmp(xchat_get_info(ph, "charset"), "UTF-8") && strcasecmp(xchat_get_info(ph, "charset"), "UTF8"))
      xchat_command(ph, "charset UTF-8");
  }
  xchat_printf(ph,"( Network is \"%s\" , \"%s\" )\n", //SparklesUser,
        SafeGet(xchat_get_info(ph, "network"),"Not found"),
        SafeGet(xchat_get_info(ph, "server"),NULL));
  return XCHAT_EAT_NONE;
}

static int TrapActionPost_cb(char *word[], char *word_eol[], void *userdata) {
  if(UseOneSayHook) {
    if(MeHook != NULL)
      xchat_unhook(ph, MeHook);
    xchat_commandf(ph, "%s %s", OneSayHook, word_eol[2]);
    MeHook = xchat_hook_command(ph, "me", XCHAT_PRI_NORM, TrapActionPost_cb, NULL, 0);
    return XCHAT_EAT_ALL;
  }
  if(strcasecmp(MeHookCommand,"")) {
    if(MeHook != NULL)
      xchat_unhook(ph, MeHook);
    xchat_commandf(ph, "%s %s", MeHookCommand, word_eol[2]);
    MeHook = xchat_hook_command(ph, "me", XCHAT_PRI_NORM, TrapActionPost_cb, NULL, 0);
    return XCHAT_EAT_ALL;
  }
  if(IsPesterchum() && !DisablePesterchum) {
    xchat_commandf(ph, "spark normalsay /me %s", word_eol[2]);
    return XCHAT_EAT_ALL;
  }
  return XCHAT_EAT_NONE;
}

static int TrapNormalPost_cb(char *word[], char *word_eol[], void *userdata) {
  if(UseOneSayHook) {
    if(SayHook != NULL)
      xchat_unhook(ph, SayHook);
    xchat_commandf(ph, "%s %s", OneSayHook, word_eol[1]);
    SayHook = xchat_hook_command(ph, "", XCHAT_PRI_NORM, TrapNormalPost_cb, NULL, 0);
    return XCHAT_EAT_ALL;
  }
  else if(CmdStackPtr != NULL) {
    char Buffer[512];
    strcpy(Buffer, CmdStackPtr);
    char *A = strstr(Buffer, "||");
    if(A) {
      *A = 0;
      CmdStackPtr=A+2;
      char *OldPtr = CmdStackPtr;
      xchat_commandf(ph, "%s %s", Buffer, word_eol[1]);
      if(OldPtr == CmdStackPtr) {
        xchat_printf(ph, "\"%s\" in cmdstack did not result in /say", Buffer);
        CmdStackPtr = NULL;
      }
      return XCHAT_EAT_ALL;
    }
    else {
      CmdStackPtr=NULL;
      xchat_commandf(ph, "%s %s", Buffer, word_eol[1]);
      return XCHAT_EAT_ALL;
    }
  }
  else {
    if(IsPesterchum() && IsChannel() && *PesterchumChanHook && !DisablePesterchum) {
      if(SayHook != NULL)
        xchat_unhook(ph, SayHook);
      xchat_commandf(ph, "%s %s", PesterchumChanHook, word_eol[1]);
      SayHook = xchat_hook_command(ph, "", XCHAT_PRI_NORM, TrapNormalPost_cb, NULL, 0);
      return XCHAT_EAT_ALL;
    }
    if(strcasecmp(SayHookCommand,"")) {
      if(SayHook != NULL)
        xchat_unhook(ph, SayHook);
      if(SayHookSpace)
        xchat_commandf(ph, "%s %s", SayHookCommand, word_eol[1]);
      else
        xchat_commandf(ph, "%s%s", SayHookCommand, word_eol[1]);
      SayHook = xchat_hook_command(ph, "", XCHAT_PRI_NORM, TrapNormalPost_cb, NULL, 0);
      return XCHAT_EAT_ALL;
    }
    return XCHAT_EAT_NONE;
  }
  return XCHAT_EAT_NONE;
}

static void ListOnEvent(char *Buffer, struct OnEventInfo *Info) {
  int j;
  *Buffer = 0;
  sprintf(Buffer, "%i", Info->Slot);
  if(Info->Flags ^ OEF_ENABLED) {
    if(!(Info->Flags) & OEF_ENABLED)
      strcat(Buffer, "d");
    if(Info->Flags & OEF_TEMPORARY)
      strcat(Buffer, "t");
    if(Info->Flags & OEF_SAVE)
      strcat(Buffer, "s");
  }
  strcat(Buffer, " \"");
  strcat(Buffer, Info->EventName);
  strcat(Buffer, "\" ");
  for(j=0;j<10;j++)
    if(Info->Match[j][0]) {
      strcat(Buffer, "\"");
      strcat(Buffer, Info->Match[j]);
      strcat(Buffer, "\" ");
    }
  strcat(Buffer, Info->Response);
}

static int AsyncExec(char *Command) {
//shamelessy taken from hexchat's exec plugin
#ifdef _WIN32
  STARTUPINFO sInfo; 
  PROCESS_INFORMATION pInfo; 
  ZeroMemory(&sInfo, sizeof (sInfo));
  ZeroMemory(&pInfo, sizeof (pInfo));
  sInfo.cb = sizeof(sInfo);
  sInfo.dwFlags = STARTF_USESTDHANDLES;
  sInfo.hStdInput = NULL;
  sInfo.hStdOutput = NULL;
  sInfo.hStdError = NULL;
  char commandLine[1024];
  sprintf(commandLine, "cmd.exe /c %s", Command);
  CreateProcess(0, commandLine, 0, 0, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 0, 0, &sInfo, &pInfo);
  return 1;
#else
  xchat_printf(ph,"AsyncExec() not ported to non-Windows platforms yet\n");
  return 0;
#endif
}
static int Spark_cb(char *word[], char *word_eol[], void *userdata) {
   char Buffer[2048]; int i=0, j=0, k=0;
   int WasValid = 0;
   if(!strcasecmp(word[2],"sprintf") || !strcasecmp(word[2],"slashf")) {
      if(word[3] != NULL) {
        sparkles_text_unescape(Buffer, word_eol[3]);
        xchat_commandf(ph,"%s", Buffer);
      }
      WasValid = 1;
   }

   if(!strcasecmp(word[2],"saveprefs")) { // save preferences back to the ini
     WasValid = 1;
     CreateDirectoriesForPath(ConfigFilePath);
     FILE *Output = fopen(ConfigFilePath,"w");
     if(!Output) {
       xchat_print(ph, "Can't open preferences file for writing\n");
       return XCHAT_EAT_ALL;
     }
     int i;
     const char *LastGroup = "";
     for(i=0;ConfigOptions[i].Group;i++) {
       if(strcmp(LastGroup, ConfigOptions[i].Group)) {
         if(*LastGroup)
           fprintf(Output, "\n");
         fprintf(Output, "[%s]\n", ConfigOptions[i].Group);
       }
       LastGroup = ConfigOptions[i].Group;

       if(ConfigOptions[i].Type == CONFIG_STRING) {
         fprintf(Output, "%s=%s\n", ConfigOptions[i].Item, (char*)ConfigOptions[i].Data);
       } else if(ConfigOptions[i].Type == CONFIG_BOOLEAN) {
         fprintf(Output, "%s=%s\n", ConfigOptions[i].Item, (*(int*)ConfigOptions[i].Data)?"yes":"no");
       } else {
         fprintf(Output, "%s=%i\n", ConfigOptions[i].Item, *(int*)ConfigOptions[i].Data);
       }
     }
     fclose(Output);
     xchat_print(ph, "Saved Sparkles preferences\n");
   }

   if(!strcasecmp(word[2],"crami")) { // cram into the input box
     WasValid = 1;
     xchat_commandf(ph, "spark onesayhook settext cram %s", word_eol[3]);
   }

   if(!strcasecmp(word[2],"cram")) { // cram multiple posts into one
     WasValid = 1;
     int Mode = word[3]!=NULL && !strcasecmp(word[3],"noslash");
     const char *Peek = xchat_get_info(ph, "inputbox");
     char *Poke = Buffer;
     const char *End = Peek+strlen(Peek);
     while(*Peek) {
       i = *(Peek++);
       if(Mode==0 && (i=='\n' || i=='\r')) {
         *(Poke++) = ' ';
         *(Poke++) = '\\';
         i=' ';
         while(*Peek == '\n' || *Peek == '\r' || *Peek == ' ')
           Peek++;
       }
       if(Mode==1 && (i=='\n' || i=='\r')) {
         while(*Peek == '\n' || *Peek == '\r' || *Peek == ' ')
           Peek++;
         if(Peek > End) break;
         continue;
       }
       *(Poke++) = i;
       if(Peek > End) break;
     }
     *Poke = 0;
     xchat_commandf(ph, "say %s\n", Buffer);
   }

   if(!strcasecmp(word[2],"mathaddi")) {
     WasValid = 1;
     for(i=3;*word[i];i++)
       j+=strtol(word[i], NULL, 10);
     xchat_commandf(ph, "settext %i", j);
   }

   if(!strcasecmp(word[2],"mathmuli")) {
     WasValid = 1;
     j=1;
     for(i=3;*word[i];i++)
       j*=strtol(word[i], NULL, 10);
     xchat_commandf(ph, "settext %i", j);
   }

   if(!strcasecmp(word[2],"silentset")) {
     WasValid = 1;
     INIConfigHandler(word[3], word[4], word_eol[5]);
   }
   if(!strcasecmp(word[2],"set")) {
     WasValid = 1;
     xchat_printf(ph, "Setting %s::%s to %s", word[3], word[4], word_eol[5]);
     INIConfigHandler(word[3], word[4], word_eol[5]);
   }
   if(!strcasecmp(word[2],"rehash")) {
     WasValid = 1;
     sprintf(Buffer, ConfigFilePath, xchat_get_info(ph, "xchatdirfs"));
     ParseINI(fopen(Buffer,"rb"), INIConfigHandler);
   }
   if(!strcasecmp(word[2],"editconfig")) {
     WasValid = 1;
     xchat_commandf(ph,"spark aexec %s %s", TextEditor, ConfigFilePath);
   }

   if(!strcasecmp(word[2],"aexec")||!strcasecmp(word[2],"asyncexec")) {
     WasValid = 1;
     AsyncExec(word_eol[3]);
   }
   if(!strcasecmp(word[2],"openlogs")) {
     WasValid = 1;
     const char *Chan = xchat_get_info(ph, "channel");
     const char *Network = xchat_get_info(ph, "network");
     const char *Server = SafeGet(xchat_get_info(ph, "server"), NULL);
     if(strcasecmp(word[3],"")) {
       Chan = word[3];
       if(strcasecmp(word[4],"")) Network = word[4];
     }
     char LogPath[100];
     const char *LogMask;
     xchat_get_prefs(ph, "irc_logmask", &LogMask, NULL);
     const char *ReplaceWith[] = {Network, Chan, Server};
     TextInterpolate(LogPath, LogMask, '%', "ncs", ReplaceWith);
   #ifdef HEXCHAT_PLUGIN_H
     sprintf(Buffer, "%s/logs/%s", xchat_get_info(ph, "xchatdirfs"), LogPath);
   #else
     sprintf(Buffer, "%s/xchatlogs/%s", xchat_get_info(ph, "xchatdirfs"), LogPath);
   #endif
     xchat_commandf(ph,"spark aexec %s %s\n", TextEditor, Buffer);
   }

   if(!strcasecmp(word[2],"repeatstring")) {
     WasValid = 1;
     *Buffer = 0;
     j = strtol(word[3],NULL,10);
     for(i=0;i<j;i++)
       strcat(Buffer, word_eol[4]);
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"useinputbox")) {
     WasValid = 1;
     xchat_commandf(ph, "%s %s", word_eol[3], SafeGet(xchat_get_info(ph, "inputbox"),NULL));
   }
   if(!strcasecmp(word[2],"sayinfo")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", SafeGet(xchat_get_info(ph, word[3]),NULL));
   }
   if(!strcasecmp(word[2],"sprintfi") || !strcasecmp(word[2],"slashfi")) { // "spark slashf settext" shortcut
     WasValid = 1;
     xchat_commandf(ph,"spark sprintf settext %s", SafeGet(word_eol[3],""));
   }
   if(!strcasecmp(word[2],"sprintfs") || !strcasecmp(word[2],"slashfs")) { // "spark slashf say" shortcut
     WasValid = 1;
     xchat_commandf(ph,"spark sprintf say %s", SafeGet(word_eol[3],""));
   }
   if(!strcasecmp(word[2],"sprintfm") || !strcasecmp(word[2],"slashfm")) { // "spark slashf me" shortcut
     WasValid = 1;
     xchat_commandf(ph,"spark sprintf me %s", SafeGet(word_eol[3],""));
   }
   if(!strcasecmp(word[2],"driving")) {
     WasValid = 1;
     xchat_commandf(ph,"say driving %i miles to %s's house %s", rand2(1000), word[3], word_eol[4]);
   }
   if(!strcasecmp(word[2],"drivingaway")) {
     WasValid = 1;
     xchat_commandf(ph,"say driving %i miles away from %s's house %s", rand2(1000), word[3], word_eol[4]);
   }
   if(!strcasecmp(word[2],"ghost2")) { // passwordless ghost with auto reclaim
     WasValid = 1;
     xchat_commandf(ph,"ns ghost %s %s", word[3], SafeGet(xchat_get_info(ph, "nickserv"),""));
     AutoReclaimNick = 1;
     strcpy(GhostReclaimNick, word[3]);
     xchat_commandf(ph,"spark spawnquiet 3 s nick %s", word[3]);
   }
   if(!strcasecmp(word[2],"ghost") && (NULL!=xchat_get_info(ph, "nickserv"))) { // passwordless ghost
     WasValid = 1;
     xchat_commandf(ph,"ns ghost %s %s", word[3], SafeGet(xchat_get_info(ph, "nickserv"),""));
   }
   if(!strcasecmp(word[2],"release") && (NULL!=xchat_get_info(ph, "nickserv"))) { // passwordless release
     WasValid = 1;
     xchat_commandf(ph,"ns release %s %s", word[3], SafeGet(xchat_get_info(ph, "nickserv"),""));
   }
   if(!strcasecmp(word[2],"ident") && (NULL!=xchat_get_info(ph, "nickserv"))) { // passwordless identify
     WasValid = 1;
     xchat_commandf(ph,"ns identify %s", SafeGet(xchat_get_info(ph, "nickserv"),""));
   }
   if(!strcasecmp(word[2],"recover") && (NULL!=xchat_get_info(ph, "nickserv"))) { // passwordless recover
     WasValid = 1;
     xchat_commandf(ph,"ns recover %s", SafeGet(xchat_get_info(ph, "nickserv"),""));
   }
#if ENABLE_NSFW_CONTENT
   if(!strcasecmp(word[2],"davyiff")) { // dav_the_fox yiff
     WasValid = 1;
     DoDavYiff(word[3]);
   }
#endif
   if(!strcasecmp(word[2],"35font")) {
     WasValid = 1;
     Print35Font(word_eol[4], word[3]);
   }
   if(!strcasecmp(word[2],"x5font")) {
     WasValid = 1;
     PrintX5Font(word_eol[4], word[3]);
   }
   if(!strcasecmp(word[2],"bigrainbow")) {
     WasValid = 1;
     xchat_commandf(ph, "spark x5font @rr01 %s", word_eol[3]);
   }
   if(!strcasecmp(word[2],"bigtext")) {
     WasValid = 1;
     xchat_commandf(ph, "spark x5font @0100 %s", word_eol[3]);
   }

   if(!strcasecmp(word[2],"normalsay")) {
     WasValid = 1;
     if(SayHook != NULL) {
       xchat_unhook(ph, SayHook);
       xchat_commandf(ph, "say %s", word_eol[3]);
       SayHook = xchat_hook_command(ph, "", XCHAT_PRI_NORM, TrapNormalPost_cb, NULL, 0);
     }
     else
       xchat_commandf(ph, "say %s", word_eol[3]);
   }
   if(!strcasecmp(word[2],"normalme")) {
     WasValid = 1;
     if(MeHook != NULL) {
       xchat_unhook(ph, MeHook);
       xchat_commandf(ph, "me %s", word_eol[3]);
       MeHook = xchat_hook_command(ph, "me", XCHAT_PRI_NORM, TrapActionPost_cb, NULL, 0);
     }
     else
       xchat_commandf(ph, "me %s", word_eol[3]);
   }

   if(!strcasecmp(word[2],"pestersay")) {
     WasValid = 1;
     char *Poke = Buffer;
     const char *Peek = xchat_get_info(ph, "nick");
     *(Poke++) = toupper(*(Peek++));
     while(*Peek) {
       if(isupper(*Peek))
         *(Poke++) = *Peek;
       Peek++;
     }
     *Poke = 0;
     xchat_commandf(ph, "say <c=%s>%s: %s</c>", PesterchumColor, Buffer, word_eol[3]);
   }
   if(!strcasecmp(word[2],"pestercolor")) {
     WasValid = 1;
     xchat_commandf(ph, "spark silentset Pesterchum Color %s", word_eol[3]);
   }

   if(!strcasecmp(word[2],"pesterchanhook")) {
     WasValid = 1;
     xchat_commandf(ph, "spark silentset Pesterchum ChannelCommand %s", word_eol[3]);
   }

   if(!strcasecmp(word[2],"sayhook") || !strcasecmp(word[2],"sayhooknospace")) {
     WasValid = 1;
     SayHookSpace = 1;
     if(!strcasecmp(word[2],"sayhooknospace"))
       SayHookSpace = 0;
     if(word_eol[3]!=NULL) {
       if(!strcasecmp(word[3],"unhook")) {
         if(SayHook != NULL) {
           xchat_unhook(ph, SayHook);
           SayHook = NULL;
           xchat_printf(ph, "%sSay unhooked\n",SparklesUser);
         }
         else
           xchat_printf(ph, "%sSay was already unhooked",SparklesUser);
       }
       else if(!strcasecmp(word[3],"hook")) {
         if(SayHook == NULL) {
           SayHook = xchat_hook_command(ph, "", XCHAT_PRI_NORM, TrapNormalPost_cb, NULL, 0);
           xchat_printf(ph, "%sSay hooked\n",SparklesUser);
         }
         else
           xchat_printf(ph, "%sSay was already hooked",SparklesUser);
       }
       else if(!strcasecmp(word[3],"off")) {
         strcpy(SayHookCommand,"");
         xchat_printf(ph, "%sSparkles will now leave /say ALONE (but the hook wasn't changed)\n",SparklesUser);
       }
       else {
         strcpy(SayHookCommand,word_eol[3]);
         xchat_printf(ph, "%s%ssay will now be interpreted as /%s\n", SparklesUser, CommandPrefix, SayHookCommand);
         if(SayHook == NULL) {
           SayHook = xchat_hook_command(ph, "", XCHAT_PRI_NORM, TrapNormalPost_cb, NULL, 0);
           xchat_printf(ph, "%sand /say was hooked again\n", SparklesUser);
         }

       }
     }
   }

   if(!strcasecmp(word[2],"mehook")) {
     WasValid = 1;
     if(word_eol[3]!=NULL) {
       if(!strcasecmp(word[3],"off")) {
         strcpy(MeHookCommand,"");
         xchat_printf(ph, "%sSparkles will now leave /me ALONE\n",SparklesUser);
       }
       else {
         strcpy(MeHookCommand,word_eol[3]);
         xchat_printf(ph, "%s%sme will now be interpreted as /%s\n", SparklesUser, CommandPrefix, MeHookCommand);
       }
     }
   }

   if(!strcasecmp(word[2],"thou")) {
     WasValid = 1;
     const char *Word1[] = {"artless", "bawdy", "beslubbering", "bootless", "churlish", "cockered", "clouted", "craven", "currish", "dankish", "dissembling", "droning", "errant", "fawning", "fobbing", "froward", "frothy", "gleeking", "goatish", "gorbellied", "impertinent", "infectious", "jarring", "loggerheaded", "lumpish", "mammering", "mangled", "mewling", "paunchy", "pribbling", "puking", "puny", "qualling", "rank", "reeky", "roguish", "ruttish", "saucy", "spleeny", "spongy", "surly", "tottering", "unmuzzled", "vain", "venomed", "villainous", "warped", "wayward", "weedy", "yeasty",NULL};
     const char *Word2[] = {"base-courted", "bat-fowling", "beef-witted", "beetle-headed", "boil-brained", "clapper-clawed", "clay-brained", "common-kissing", "crook-pated", "dismal-dreaming", "dizzy-eyed", "doghearted", "dread-bolted", "earth-vexing", "elf-skinned", "fat-kidneyed", "fen-sucked", "flap-mouthed", "fly-bitten", "folly-fallen", "fool-born", "full-gorged", "guts-griping", "half-faced", "hasty-witted", "hedge-born", "hell-hated", "idle-headed", "ill-breeding", "ill-nurtured", "knotty-pated", "milk-livered", "motley-minded", "onion-eyed", "plume-plucked", "pottle-deep", "pox-marked", "reeling-ripe", "rough-hen", "rude-growing", "rump-fed", "sheep-biting", "spur-galled", "swag-bellied", "tardy-gaited", "tickle-brained", "toad-spotted", "urchin-snouted", "weather-bitten",NULL};
     const char *Word3[] = {"apple-john", "baggage", "barnacle", "bladder", "boar-pig", "bugbear", "bum-bailey", "canker-blossom", "clack-dick", "clotpole", "coxcomb", "codpiece", "death-token", "dewberry", "flap-dragon", "flax-wench", "flirt-gill", "foot-licker", "fustilarian", "giglet", "gudgeon", "haggard", "harpy", "hedge-pig", "horn-beast", "hugger-bugger", "joithead", "lewdster", "lout", "maggot-pie", "malt-worm", "mammet", "measle", "minnow", "miscreant", "moldwarp", "mumble-news", "nut-hook", "pigeon-egg", "pignut", "puttock", "pumpion", "ratsbane", "scut", "skainsmate", "strumpet", "varlot", "vassal", "wheyface", "wagtail",NULL};
     int Word1L=0, Word2L=0, Word3L=0;
     while(Word1[Word1L] != NULL) Word1L++;
     while(Word2[Word2L] != NULL) Word2L++;
     while(Word2[Word3L] != NULL) Word3L++;
     if(strcasecmp(word[3],"")&&strlen(word[3]))
       xchat_commandf(ph, "say %s, thou %s %s %s!", word[3], Word1[rand2(Word1L)], Word2[rand2(Word2L)], Word3[rand2(Word3L)]);
     else
       xchat_commandf(ph, "say Thou %s %s %s!", Word1[rand2(Word1L)], Word2[rand2(Word2L)], Word3[rand2(Word3L)]);
   }

   if(!strcasecmp(word[2],"rot13")) {
     WasValid = 1;
     strcpy(Buffer,word_eol[3]);
     char offset;
     for(i=0;Buffer[i];i++) {
       if(!isalpha(Buffer[i]))
         continue;
       if(islower(Buffer[i]))
         offset = 'a';
       else
         offset = 'A';
       if(Buffer[i] - offset < 13)
         Buffer[i] += 13;
       else
         Buffer[i] -= 13;
     }
     xchat_commandf(ph, "say %s", Buffer);
   }

   if(!strcasecmp(word[2],"cmdstack") || !strcasecmp(word[2],"pipe")) {
     WasValid = 1;
     strcpy(CmdStackText, word[3]);
     CmdStackPtr = CmdStackText;
     strcpy(Buffer, CmdStackText);
     char *A = strstr(Buffer, "||");
     if(A) {
       *A = 0;
       CmdStackPtr=A+2;
       xchat_commandf(ph, "%s %s", Buffer, word_eol[4]);
     }
   }

   if(!strcasecmp(word[2],"multicmd")) {
     WasValid = 1;
     char *Ptr = word[3];
     while(1) {
       strcpy(Buffer, Ptr);
       char *A = strstr(Buffer, "||");
       if(A) {
         *A = 0;
         Ptr=A+2;
         xchat_commandf(ph, "%s", Buffer);
       }
       else {
         xchat_commandf(ph, "%s", Buffer);
         break;
       }
     }
   }

   if(!strcasecmp(word[2],"SparkEncrypt1")) {
     WasValid = 1;
     unsigned char Key1 = rand()&255;
     unsigned char kA = (Key1 & 192) >> 6;
//   unsigned char kB = (Key1 & 48) >> 4;
     unsigned char kC = (Key1 & 15);
     char TinyBuf[128];
     sprintf(TinyBuf,"%s\3%.2d\3%.2d\3%.2d\xf",SparkEncryptPrefix,
       Key1&15, (Key1&0xf0)>>4, 0&15);
     strcpy(Buffer,TinyBuf);

     char *Poke = Buffer+strlen(TinyBuf);
     char *Peek = word_eol[3];

     while(*Peek) {
       char c = *(Peek++);
       char nyb = c &15;
       c&=~0x0f;
       if(c>=0x20) {
         switch(kA) {
           case 0: nyb ^=15;
           case 1: nyb +=kC; break;
           case 2: nyb ^=15;
           case 3: nyb -=kC; break;
         }
       }
       c|=(nyb&15);
       *(Poke++) = c;
     }
     *Poke = 0;
     if(NULL != strstr(Buffer, word_eol[3]))
       xchat_commandf(ph,"spark sparkencrypt1 %s", word_eol[3]);
     else {
//       xchat_printf(ph,"(SE1 %2i %2i)-\t%s\n", kA, kC, word_eol[3]);
       xchat_printf(ph,"(SE1)-\t%s\n", word_eol[3]);
       xchat_commandf(ph,"say %s", Buffer);
     }
   }

   if(!strcasecmp(word[2],"2sprintf") || !strcasecmp(word[2],"2slashf") || !strcasecmp(word[2],"2slashfi")) {
     WasValid = 1;
     char TinyBuf[32];
     char *Poke = Buffer;
     char *Peek = word_eol[3];
     char c;
     while(*Peek) {
       c = *(Poke++) = *(Peek++);
       switch(c) {
         case 0x1f: Poke--; *(Poke++) = '\\'; *(Poke++) = 'u';  break;
         case 0x02: Poke--; *(Poke++) = '\\'; *(Poke++) = 'b';  break;
         case 0x03: Poke--; *(Poke++) = '\\'; *(Poke++) = 'c';  break;
         case 0x1d: Poke--; *(Poke++) = '\\'; *(Poke++) = 'i';  break;
         case 0x16: Poke--; *(Poke++) = '\\'; *(Poke++) = 'R';  break;
         case 0x0f: Poke--; *(Poke++) = '\\'; *(Poke++) = 'p';  break;
         case 0x07: Poke--; *(Poke++) = '\\'; *(Poke++) = 'a';  break;
         case '\\': *(Poke++) = '\\'; break;
         default:
           if(!(isgraph2(c)||c==' ')) {
             *(--Poke) = 0;
             sprintf(TinyBuf, "\\x%x ", c);
             strcat(Buffer, TinyBuf);
             Poke+=strlen(TinyBuf);
           }
           break;
       }
     }
     *Poke = 0;
     if(strcasecmp(word[2],"2slashfi"))
       xchat_printf(ph, "%s%sspark slashfs %s\n", SparklesUser, CommandPrefix, Buffer);
     else {
       xchat_commandf(ph, "settext /spark slashfs %s", Buffer);
       xchat_commandf(ph, "setcursor 15");
     }
   }

   if(!strcasecmp(word[2],"atloop")) { // atloop [format] [action] [text]
     WasValid = 1;
     if(!AttributeLoop(Buffer, word_eol[5], word[3])) {
       xchat_commandf(ph, "%s %s", word[4], Buffer);
     } else {
       xchat_printf(ph, "%sBad attribute loop?\n",SparklesUser);
     }
   }
   if(!strcasecmp(word[2],"atloops")) {
     WasValid = 1;
     xchat_commandf(ph,"spark atloop %s say %s", SafeGet(word[3],""), SafeGet(word_eol[4],""));
   }
   if(!strcasecmp(word[2],"atloopm")) {
     WasValid = 1;
     xchat_commandf(ph,"spark atloop %s me %s", SafeGet(word[3],""), SafeGet(word_eol[4],""));
   }
   if(!strcasecmp(word[2],"atloopi")) {
     WasValid = 1;
     xchat_commandf(ph,"spark atloop %s settext %s", SafeGet(word[3],""), SafeGet(word_eol[4],""));
   }
   if(!strcasecmp(word[2],"unsettab")) {
     WasValid = 1;
     xchat_commandf(ph,"settab %s", SafeGet(xchat_get_info(ph, "channel"),""));
   }

   if(!strcasecmp(word[2],"backwards")) {
     WasValid = 1;
     char *Poke = Buffer;
     for(i=strlen(word_eol[3])-1;i!=-1;i--)
       *(Poke++) = word_eol[3][i];
     *Poke = 0;
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"backwords")) {
     WasValid = 1;
     Backwords(word_eol[3], Buffer);
     xchat_commandf(ph, "say %s", Buffer);
   }

   if(!strcasecmp(word[2],"grabtopic")) {
     WasValid = 1;
     GrabbingTopic = 1;
     xchat_command(ph, "topic");
   }

   if(!strcasecmp(word[2],"spawn") || !strcasecmp(word[2],"spawnquiet")) {
     WasValid = 1;
     // /spark(1) spawn(2) X(3) units(4) Command(5)
     int Slot;
     if(!strcasecmp(word[3],"clear")) {
       for(Slot=0;Slot<16;Slot++)
         if(SpawnHook[Slot]!=NULL) {
           struct SpawnInfo *FreeMe = xchat_unhook(ph, SpawnHook[Slot]);
           if(FreeMe != NULL)
             free(FreeMe);
           SpawnHook[Slot]=NULL;
         }
       if(strcasecmp(word[2],"spawnquiet"))
         xchat_printf(ph, "%sSpawn list cleared",SparklesUser);
    } else {
      double Time = strtod(word[3], NULL);
      switch(word[4][0]) {
         case 'd':
           Time*=24;
         case 'h':
           Time*=60;
         case 'm':
           Time*=60;
         case 's':
           Time*=1000;
           break;
       }

       struct SpawnInfo *Spawn = (struct SpawnInfo*)malloc(sizeof(struct SpawnInfo));
       int SpawnUsed = 0;
       for(Slot = 0; Slot<64; Slot++)
         if(SpawnHook[Slot] == NULL)
           if(Spawn != NULL) {
             if(strcasecmp(word[2],"spawnquiet"))
               xchat_printf(ph,"%sTimer started! (%f seconds) \n", SparklesUser, Time/1000);
             strcpy(Spawn->Command, word_eol[5]);
             Spawn->Context = xchat_get_context(ph);
             Spawn->Slot = Slot;
             SpawnHook[Slot] = xchat_hook_timer(ph, Time, spawntimer_cb, Spawn);
             SpawnUsed = 1;
             break;
           }
       if(!SpawnUsed)
         free(Spawn);
     }
   }

   if(!strcasecmp(word[2],"rainbow") || !strcasecmp(word[2],"rainbowt")) {
     WasValid = 1;
     char *Poke = Buffer;
      for(i=0,j=0;word_eol[3][i];i++) {
        if(isgraph2(word_eol[3][i])) {
          *(Poke++) = 0x03;
          *(Poke++) = Rainbow[j][0];
          *(Poke++) = Rainbow[j++][1];
          if(j == 5) j = 0;
        }
        *(Poke++) = word_eol[3][i];
      }
      if(strcasecmp(word[2],"rainbowt"))
        *Poke = 0;
      else {
        *(Poke++) = 0x0f;
        *Poke = 0;
      }
      xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"hstroll")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", TrollFilter(Buffer, word_eol[4], word[3]));
   }
   if(!strcasecmp(word[2],"scarletsay")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", ScarletFilter(Buffer, word_eol[3]));
   }
   if(!strcasecmp(word[2],"sellysay")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", SellyFilter(Buffer, word_eol[3], 0));
   }
   if(!strcasecmp(word[2],"sellysay2")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", SellyFilter(Buffer, word_eol[3], 1));
   }
   if(!strcasecmp(word[2],"replwords")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", CustomReplace(Buffer, word_eol[4], word[3]));
   }
   if(!strcasecmp(word[2],"replchars") || !strcasecmp(word[2],"replcharsi") || !strcasecmp(word[2],"replcharsim")) {
      WasValid = 1;
      if(strlen(word[3])&1) {
        xchat_printf(ph, "The list given to replchars must be of an even length\n");
        return XCHAT_EAT_ALL;
      }
      int CaseSensitive = !strcasecmp(word[2],"replchars");
      int MatchCase = !strcasecmp(word[2],"replcharsim");

      strcpy(Buffer, word_eol[4]);
      char *Poke = Buffer;
      while(*Poke) {
        char *RList = word[3];
        while(*RList) {
          if(*Poke == *RList)
            *Poke = RList[1];
          else if(!CaseSensitive && toupper(*Poke) == toupper(*RList)) {
            if(!MatchCase || !isalpha(*Poke))
              *Poke = RList[1];
            else
              *Poke = isupper(*Poke)?toupper(RList[1]):tolower(RList[1]);
          }
          RList+=2;
        }
        Poke++;
      }

      xchat_commandf(ph, "say %s", Buffer);
   }

   if(!strcasecmp(word[2],"onesayhook") || !strcasecmp(word[2],"me2cmd") || !strcasecmp(word[2],"say2cmd")) {
     WasValid = 1;
     strcpy(OneSayHook, word[3]);
     UseOneSayHook = 1;
     xchat_command(ph, word_eol[4]);
     UseOneSayHook = 0;
   }

   if(!strcasecmp(word[2],"me2say")) {
     WasValid = 1;
     strcpy(OneSayHook, "say");
     UseOneSayHook = 1;
     xchat_command(ph, word_eol[3]);
     UseOneSayHook = 0;
   }

   if(!strcasecmp(word[2],"hideset")) {
     WasValid = 1;
     xchat_commandf(ph, "spark hidecmd set %s");
   }

   if(!strcasecmp(word[2],"hidecmd")) {
     WasValid = 1;
     xchat_context *Old = xchat_get_context(ph);
     xchat_command(ph, "query $");
     xchat_context *New = xchat_find_context(ph, NULL, "$");
     if(New != NULL)
       if(xchat_set_context(ph, New)) {
         xchat_command(ph, word_eol[3]);
         xchat_command(ph, "close");
         xchat_set_context(ph, Old);
         xchat_command(ph, "gui focus");
       }
   }

   if(!strcasecmp(word[2],"space2newline")) {
     WasValid = 1;
     char *Ptr = word_eol[3];
     while(1) {
       strcpy(Buffer, Ptr);
       char *A = strstr(Buffer, " ");
       if(A) {
         *A = 0;
         Ptr=A+1;
         xchat_commandf(ph, "say %s", Buffer);
       }
       else {
         xchat_commandf(ph, "say %s", Buffer);
         break;
       }
     }
   }

   if(!strcasecmp(word[2],"spaces2newline")) {
     WasValid = 1;
     int SpaceCount = 0;
     int WordsPerLine = strtol(word[3], NULL, 10);
     if(WordsPerLine < 1)
       return XCHAT_EAT_ALL;
     strcpy(Buffer, word_eol[4]);
     char *Ptr = Buffer;
     char *Send = Ptr;
     while(*Ptr) {
       if(*Ptr == ' ') {
         SpaceCount++;
         if(SpaceCount == WordsPerLine) {
           *Ptr = 0;
           xchat_commandf(ph, "say %s", Send);
           Send = Ptr+1;
           SpaceCount = 0;
         }
       }
       Ptr++;
     }
     if(*Send)
       xchat_commandf(ph, "say %s", Send);
   }

   if(!strcasecmp(word[2],"repeatcmd")) { // /spark repeatcmd times command
     WasValid = 1;
     int i, Times=strtol(word[3],NULL,10);
     char *Cmd = word_eol[4];
     if(isdigit(word[4][0])) {
       Cmd = word_eol[5];
       xchat_commandf(ph, "%s", Cmd);
       if(Times>1)
         xchat_commandf(ph, "spark spawnquiet %s s spark repeatcmd %i %s %s", word[4], Times-1, word[4], Cmd);
     }
     else
       for(i=0;i<Times;i++)
         xchat_commandf(ph, "%s", Cmd);
   }
   if(!strcasecmp(word[2],"rainbow4") || !strcasecmp(word[2],"rainbow4t")) { // 04 08 09 12 13
     WasValid = 1;
     char *Poke = Buffer;
     for(i=0,j=0;word_eol[3][i];i++) {
       if(isgraph2(word_eol[3][i]) && !(i&3)) {
         *(Poke++) = 0x03;
         *(Poke++) = Rainbow[j][0];
         *(Poke++) = Rainbow[j++][1];
         if(j == 5) j = 0;
       }
       *(Poke++) = word_eol[3][i];
     }
     if(strcasecmp(word[2],"rainbow4t"))
       *(Poke++) = 0x0f;
     *Poke = 0;
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"bouncycaps")) {
     WasValid = 1;
     strcpy(Buffer,word_eol[3]);
     Buffer[0]=toupper(Buffer[i]);
     for(i=0;Buffer[i];i++)
       if(Buffer[i] == ' ')
         Buffer[i+1]=toupper(Buffer[i+1]);
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"shufflechar")) { // /spark shufflechar Corruption Text
     WasValid = 1;
     ShuffleChars(Buffer, word_eol[4],  strtol(word[3],NULL,10),0);
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"allcaps")) {
     WasValid = 1;
     strcpy(Buffer,word_eol[3]);
     for(i=0;Buffer[i];i++)
       Buffer[i]=toupper(Buffer[i]);
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"nocaps")) {
     WasValid = 1;
     strcpy(Buffer,word_eol[3]);
     for(i=0;Buffer[i];i++)
       Buffer[i]=tolower(Buffer[i]);
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"altcaps")) {
     WasValid = 1;
     strcpy(Buffer,word_eol[3]);
     for(i=0;Buffer[i];i++)
       if(i&1)
         Buffer[i]=toupper(Buffer[i]);
       else
         Buffer[i]=tolower(Buffer[i]);
     xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"acidtext")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", AcidText(Buffer, word_eol[3]));
   }
   if(!strcasecmp(word[2],"accents")) {
     WasValid = 1;
     xchat_commandf(ph, "say %s", AccentFilter(Buffer, word_eol[3]));
   }
   if(!strcasecmp(word[2],"randcaps") || !strcasecmp(word[2],"rainbowcaps")) {
     WasValid = 1;
     strcpy(Buffer,word_eol[3]);
     for(i=0;Buffer[i];i++)
       if(rand()&1)
         Buffer[i]=toupper(Buffer[i]);
       else
         Buffer[i]=tolower(Buffer[i]);
     if(!strcasecmp(word[2],"rainbowcaps"))
       xchat_commandf(ph, "spark rainbow %s", Buffer);
     else
       xchat_commandf(ph, "say %s", Buffer);
   }
   if(!strcasecmp(word[2],"run")) { // take a file and execute every line of it as a command
      WasValid = 1;
      int Quiet=0;
      if(word[4] != NULL && !strcasecmp(word[4],"quiet"))
        Quiet=1;
      if(word[3] != NULL) {
        FILE *Script;
        Script = fopen(word[3],"r");
        if(Script == NULL) {
          xchat_printf(ph, "%sUnable to open \"%s\" for running\n", SparklesUser, word[3]);
          return XCHAT_EAT_ALL; 
        }
        if(BeVerbose)
          xchat_printf(ph, "%s'kay, opened \"%s\"\n", SparklesUser, word[3]);
        for(k=0;!k;i=0) {
          i=0;
          while(1) {
            j = fgetc(Script);
            Buffer[i++]=j;
            if(j == '\n' || j==EOF) {
               Buffer[i-1] = 0;
               if(j==EOF) k=1;
               break;
            }
          }
          if(!Quiet)
            xchat_printf(ph, "%sread \"%s\"\n", SparklesUser,Buffer);
          xchat_commandf(ph, "%s", Buffer);
        }
        fclose(Script);
     }
   }

   if(!strcasecmp(word[2],"onevent")){
     WasValid = 1;
     if(!strcasecmp(word[3], "list")) {
       for(i=0;i<ONEVENTS_SIZE;i++)
         if(OnEventInfos[i]) {
           ListOnEvent(Buffer, OnEventInfos[i]);
           xchat_print(ph, Buffer);
         }
     } else if(!strcasecmp(word[3], "renumber")) {
       for(i=0;i<ONEVENTS_SIZE-1;i++) {
         if(OnEventInfos[i])
           continue;
         for(j=i+1;j<ONEVENTS_SIZE;j++)
           if(OnEventInfos[j]) {
             OnEventInfos[i] = OnEventInfos[j];
             OnEventInfos[j] = NULL;
             OnEventInfos[i]->Slot = i;
           }
       }
       xchat_print(ph, "Renumbered the OnEvent list");
     } else if(!strcasecmp(word[3], "delete")) {
       if(!strcasecmp(word[4], "all")) {
         for(i=0;i<ONEVENTS_SIZE;i++)
           if(OnEventInfos[i]) {
             xchat_unhook(ph, OnEventInfos[i]->Hook);
             free(OnEventInfos[i]);
             OnEventInfos[i] = NULL;
           }
       } else {
         for(i=4;word[i][0];i++) {
           j = strtol(word[i], NULL, 10);
           if(OnEventInfos[j]) {
             xchat_unhook(ph, OnEventInfos[j]->Hook);
             free(OnEventInfos[j]);
             OnEventInfos[j] = NULL;
             if(!QuietOnEvents)
               xchat_printf(ph, "Deleting OnEvent item %i", j);
           }
         }
       }
     } else if(!strcasecmp(word[3], "disable")) {
       for(i=4;word[i][0];i++) {
         j = strtol(word[i], NULL, 10);
         if(OnEventInfos[j]) {
           OnEventInfos[j]->Flags &= ~OEF_ENABLED;
           if(!QuietOnEvents)
             xchat_printf(ph, "Disabling OnEvent item %i", i);
         }
       }
     } else if(!strcasecmp(word[3], "enable")) {
       for(i=4;word[i][0];i++) {
         j = strtol(word[i], NULL, 10);
         if(OnEventInfos[j]) {
           OnEventInfos[j]->Flags |= OEF_ENABLED;
           if(!QuietOnEvents)
             xchat_printf(ph, "Enabling OnEvent item %i", i);
         }
       }
     } else if(!strcasecmp(word[3], "set")) {
       j = -1;
       if(isdigit(word[4][0])) // user selected a specific slot
         j = strtol(word[4], NULL, 10);
       if(j == -1) {
         for(j=0;j<ONEVENTS_SIZE;j++)
           if(!OnEventInfos[j])
             break;
         if(j==ONEVENTS_SIZE) {
           xchat_print(ph, "No free OnEvent slots");
           return XCHAT_EAT_ALL;
         }
       }
       if(OnEventInfos[j]) {
         xchat_unhook(ph, OnEventInfos[j]->Hook);
         free(OnEventInfos[j]);
       }
       OnEventInfos[j] = (struct OnEventInfo*)malloc(sizeof(struct OnEventInfo));
       if(!OnEventInfos[j])
         return XCHAT_EAT_ALL;
       memset(OnEventInfos[j], 0, sizeof(struct OnEventInfo));
       strlcpy(OnEventInfos[j]->EventName, word[5], 32);
       OnEventInfos[j]->Slot = j;
       OnEventInfos[j]->Flags = OEF_ENABLED;
       int High = 0;
       for(k=0;word[4][k];k++) {
         switch(word[4][k]) {
           case 't':
             OnEventInfos[j]->Flags |= OEF_TEMPORARY;
             break;
           case 'd':
             OnEventInfos[j]->Flags &= ~OEF_ENABLED;
             break;
           case 's':
             OnEventInfos[j]->Flags |= OEF_SAVE;
             break;
           case 'h':
             OnEventInfos[j]->Flags |= OEF_HIGH_PRIORITY;
             High = 1;
             break;
         }
       }
       for(k=6;isdigit(word[k][0]);k++)
         strlcpy(OnEventInfos[j]->Match[k-6], word[6], 80);
       strlcpy(OnEventInfos[j]->Response, word_eol[k], 500);
       OnEventInfos[j]->Hook = xchat_hook_print(ph, word[5], High?XCHAT_PRI_HIGH:XCHAT_PRI_NORM, on_event_cb, OnEventInfos[j]); 
       xchat_printf(ph, "Set OnEvent slot %i", j);
     }
   }

   if(!strcasecmp(word[2],"contextstack")){
     WasValid = 1;
     if(!strcasecmp(word[3],"push"))
       ContextStack[ContextStackSP++] = xchat_get_context(ph);
     else if(!strcasecmp(word[3],"pop"))
       xchat_set_context(ph,ContextStack[--ContextStackSP]);
     else if(!strcasecmp(word[3],"clear"))
       ContextStackSP=0;
     else
       xchat_printf(ph, "Contextstack is at position %i",ContextStackSP);
   }

   if(!strcasecmp(word[2],"cmdallchan")){
     WasValid = 1;
	 xchat_list *list = xchat_list_get(ph, "channels");
     int DoInChannels = 0;
     int DoInQueries = 0;
     int DoInServers = 0;
     int ThisNetworkOnly = 0;
     int PartedOnly = 0;
     int Id;
     char *a = word[3];
     while(*a) {
       switch(*(a++)) {
         case 'N': ThisNetworkOnly = 1; break;
         case 'q': DoInQueries = 1; break;
         case 'c': DoInChannels = 1; break;
         case 's': DoInServers = 1; break;
         case 'p': PartedOnly = 1; break;
         case 'P': PartedOnly = 2; break;
       }
     }
     const char *b;
     xchat_get_prefs(ph, "id", &b, &Id);
	 if(list) {
       while(xchat_list_next(ph, list)) {
         if(xchat_list_int(ph, list, "type")==2) { // channel
           if(DoInChannels)
             if(PartedOnly==2 || (PartedOnly==1&&!xchat_list_int(ph, list, "users")) || (PartedOnly==0&&xchat_list_int(ph, list, "users")))
               if(!ThisNetworkOnly || xchat_list_int(ph, list, "id")==Id)
                 if(xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context")) )
                   xchat_command(ph, word_eol[4]);
         }
         else if(xchat_list_int(ph, list, "type")==1) { // server
           if(DoInServers)
             if(xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context")) )
               xchat_command(ph, word_eol[4]);
         }
         else if(xchat_list_int(ph, list, "type")==3) { // query
           if(DoInQueries)
             if(!ThisNetworkOnly || xchat_list_int(ph, list, "id")==Id)
               if(xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context")) )
                 xchat_command(ph, word_eol[4]);
         }
       }
       xchat_list_free(ph, list);
     }
   }
   if(!strcasecmp(word[2],"chanclean")) {
     WasValid = 1;
     xchat_commandf(ph, "spark cmdallchan %sp close", word[3]);
   }

   if(!strcasecmp(word[2],"SendModeSelect")){
     WasValid = 1;
     xchat_list *list;
     int Users = 0;
     const char *UserList[100];
     list = xchat_list_get(ph, "users");
     if(list){
       while(xchat_list_next(ph, list)) {
          if(xchat_list_int(ph, list, "selected")) {
            UserList[Users]=xchat_list_str(ph, list, "nick");
            Users = Users+1;
            if(Users >= 100)
              break;
          }
       }
       xchat_list_free(ph, list);
     }
//     xchat_printf(ph, "Setting %s on %i users\n", word[3], Users);
     xchat_send_modes(ph, UserList, Users, 0, word[3][0], word[3][1]);
   }

   if(!strcasecmp(word[2],"SendModePrefix")){ // /spark sendmodeprefix +o prefix exclude
     WasValid = 1;
     xchat_list *list;
     int Users = 0;
     int Exclude;
     char *Prefix = word[4];
     if(!strcasecmp(Prefix, "."))
       Prefix = "";
     const char *UserList[100]; // nobody will ever trust this evil squirrel with >40 users
     list = xchat_list_get(ph, "users");
     if(list){
       while(xchat_list_next(ph, list)) {
          if(!strcasecmp(Prefix, xchat_list_str(ph, list, "prefix"))) {
            int ShouldExclude = 0;
            for(Exclude = 5; word[Exclude]!=NULL && strlen(word[Exclude]); Exclude++)
              if(!strcasecmp(word[Exclude], xchat_list_str(ph, list, "nick"))) {
                ShouldExclude = 1;
                break;
              }
            if(ShouldExclude)
              continue;
            UserList[Users]=xchat_list_str(ph, list, "nick");
            Users = Users+1;
            if(Users >= 100)
              break;
          }
       }
       xchat_list_free(ph, list);
     }
     xchat_send_modes(ph, UserList, Users, 0, word[3][0], word[3][1]);
   }

   if(!strcasecmp(word[2],"pusers")){ // prefix of users in local context
     WasValid = 1;
     xchat_list *list;
     int Users = 0;
     list = xchat_list_get(ph, "users");
     if(list){
       while(xchat_list_next(ph, list)) {
          xchat_printf(ph, "user %s has %s \n",xchat_list_str(ph, list, "nick"), xchat_list_str(ph, list, "prefix"));
          Users = Users+1;
       }
       xchat_list_free(ph, list);
     }
   }

   if(!strcasecmp(word[2],"get_info")) { // for debugging and curiosity
      WasValid = 1;
      if(word[3] != NULL)
        xchat_printf(ph,"%sxchat_get_info(ph, \"%s\") returns \"%s\";\n", SparklesUser,
        word[3], SafeGet(xchat_get_info(ph, word[3]),NULL));
   }
   if(!strcasecmp(word[2],"get_prefs")) { // for debugging and curiosity
      WasValid = 1;
      int Int;
      const char *Str;
      switch(xchat_get_prefs (ph, word[3], &Str, &Int)) {
        case 0:
          xchat_printf(ph, "%sxchat_get_prefs() failed\n" ,SparklesUser);
          break;
        case 1:
          xchat_printf(ph, "%sstring: \"%s\" \n", SparklesUser, Str);
          break;
        case 2:
          xchat_printf(ph, "%sinteger: %i | %x \n", SparklesUser, Int, Int);
          break;
        case 3:
          xchat_printf(ph, "%sboolean: %i \n", SparklesUser, Int);
          break;
      }
   }

   if(!strcasecmp(word[2],"randtype")) {
     WasValid = 1;
     RandomType = strtol(word[3], NULL, 10);
     if(word[4] == NULL || strcasecmp(word[4],"quiet"))
       xchat_printf(ph, "%sRandom number type set to %i \n", SparklesUser, RandomType);
   }
   if(!strcasecmp(word[2],"randnum")) {
     WasValid = 1;
     xchat_printf(ph, "%sRandom number: %i \n", SparklesUser, rand2(strtol(word[3], NULL, 10)));
   }

   if(!strcasecmp(word[2],"pesterchum")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset Pesterchum Disabled on");
       xchat_printf(ph,"%sPesterchum mode OFF",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset PrettyJanus Disabled off");
       xchat_printf(ph,"%sPesterchum mode ON",SparklesUser);
     }
   }
   if(!strcasecmp(word[2],"prettyjanus")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset PrettyJanus Disabled on");
       xchat_printf(ph,"%sPretty Janus OFF",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset PrettyJanus Disabled off");
       xchat_printf(ph,"%sPretty Janus ON",SparklesUser);
     }
   }
   if(!strcasecmp(word[2],"invitejoin")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset Automatic JoinOnInvite on");
       xchat_printf(ph,"%sJoin-on-invite enabled",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset Automatic JoinOnInvite off");
       xchat_printf(ph,"%sJoin-on-invite disabled",SparklesUser);
     }
   }
   if(!strcasecmp(word[2],"rejoin")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset Automatic RejoinOnKick on");
       xchat_printf(ph,"%sRejoin-on-kick enabled",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset Automatic RejoinOnKick off");
       xchat_printf(ph,"%sRejoin-on-kick disabled",SparklesUser);
     }
   }
   if(!strcasecmp(word[2],"autoident")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset Automatic DisableAutoIdent on");
       xchat_printf(ph,"%sAuto-ident disabled",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset Automatic DisableAutoIdent off");
       xchat_printf(ph,"%sAuto-ident re-enabled",SparklesUser);
     }
   }

   if(!strcasecmp(word[2],"autoghost")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset Automatic DisableAutoGhost on");
       xchat_printf(ph,"%sAuto-ghost disabled",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset Automatic DisableAutoGhost on");
       xchat_printf(ph,"%sAuto-ghost re-enabled",SparklesUser);
     }
   }

   if(!strcasecmp(word[2],"autonickdeblue")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset Automatic DisableAutoNickColorReset on");
       xchat_printf(ph,"%sAuto-nick color reset disabled",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset Automatic DisableAutoNickColorReset off");
       xchat_printf(ph,"%sAuto-nick color reset re-enabled",SparklesUser);
     }
   }

   if(!strcasecmp(word[2],"plusjfix")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       DisablePlusJFix = 1; xchat_printf(ph,"%s+J fix disabled",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       DisablePlusJFix = 0; xchat_printf(ph,"%s+J fix re-enabled",SparklesUser);
     }
   }

   if(!strcasecmp(word[2],"activity2focus")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       xchat_commandf(ph, "spark silentset Automatic Activity2Focus on");
       xchat_printf(ph,"%sActivity-to-Focus enabled",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       xchat_commandf(ph, "spark silentset Automatic Activity2Focus off");
       xchat_printf(ph,"%sActivity-to-Focus disabled",SparklesUser);
     }
   }

   if(!strcasecmp(word[2],"eathighlights")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"on")) {
       EatHighlights = 1; xchat_printf(ph,"%sHighlights will now be eaten",SparklesUser);
     }
     if(!strcasecmp(word[3],"off")) {
       EatHighlights = 0; xchat_printf(ph,"%sHighlights will now be left alone",SparklesUser);
     }
   }

   if(!strcasecmp(word[2],"invite")) {
     WasValid = 1;
     if(!strcasecmp(word[3],"eat")) {
       EatInvites = 1;  xchat_printf(ph,"%sInvites will now be eaten",SparklesUser);
     }
     if(!strcasecmp(word[3],"leave")) {
       EatInvites = 0;  xchat_printf(ph,"%sInvites will now be left alone",SparklesUser);
     }
   }
   if(!strcasecmp(word[2],"unsettabs")) { // unset all tabs
     WasValid = 1;
     xchat_list *list = xchat_list_get(ph, "channels");
     if(list) {
       while(xchat_list_next(ph, list)) {
         if( xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context")))
           xchat_command(ph, "spark unsettab");
       }
       xchat_list_free(ph, list);
     }
   }
   if(!strcasecmp(word[2],"rchan")) { // with preserved context
     WasValid = 1;
     xchat_command(ph, "spark contextstack push");
     xchat_commandf(ph, "spark rchanc %s", word_eol[3]);
     xchat_command(ph, "spark contextstack pop");
   }
   if(!strcasecmp(word[2],"rchan2")) { // with preserved context
     WasValid = 1;
     xchat_command(ph, "spark contextstack push");
     xchat_commandf(ph, "spark rchan2c %s", word_eol[3]);
     xchat_command(ph, "spark contextstack pop");
   }
   if(!strcasecmp(word[2],"rchan2c")) { // redirect to channel
     WasValid = 1;
     xchat_list *list = xchat_list_get(ph, "channels");
     if(list) {
       while(xchat_list_next(ph, list)) {
         if(!strcasecmp(xchat_list_str(ph, list, "channel"),SafeGet(word[3],NULL))) {
           if( xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context")))
             xchat_command(ph, SafeGet(word_eol[4],"echo Okay, and do WHAT on that channel?"));
         }
       }
       xchat_list_free(ph, list);
     }
   }
   if(!strcasecmp(word[2],"rchanc")) { // redirect to channel (on same network)
     WasValid = 1;
     xchat_context *Go = xchat_find_context(ph, NULL, word[3]);
     if( xchat_set_context(ph, Go))
       xchat_command(ph, word_eol[4]);
   }
   if(!strcasecmp(word[2],"mrchan")) {
     WasValid = 1;
     char *Ptr = word[3];
     while(1) {
       strcpy(Buffer, Ptr);
       char *A = strstr(Buffer, "!");
       if(A) {
         *A = 0;
         Ptr=A+1;
         xchat_context *Go = xchat_find_context(ph, NULL, Buffer);
         if(xchat_set_context(ph, Go))
           xchat_command(ph, word_eol[4]);
       }
       else {
         xchat_context *Go = xchat_find_context(ph, NULL, Buffer);
         if(xchat_set_context(ph, Go))
           xchat_command(ph, word_eol[4]);
         break;
       }
     }
   }

   if(!strcasecmp(word[2],"help")) { // get help
     WasValid = 1;
     xchat_print(ph,"Read https://github.com/NovaSquirrel/SparklesPlugin/blob/master/README.md");
   }
/* WHY DID I EVEN TRY TO IMPLEMENT THIS
   AND WHY DIDN'T I JUST MAKE IT READ WHOIS RESULTS
   if(!strcasecmp(word[2],"mutualchan")) {
     WasValid = 1;
     strcpy(Buffer,"");
     if(!strcasecmp(word[3],"")) {
        // use current channel
     }
     else { // search for channels you're sharing with the given user
       xchat_list *list = xchat_list_get(ph, "channels");
       if(list != NULL) {
          while(xchat_list_next(ph, list))
            if(xchat_list_int(ph, list, "type")==2 && xchat_list_int(ph, list, "users"))
              if(xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context"))) {
                xchat_list *users = xchat_list_get(ph, "users");
                if(users != NULL) {
                  while(xchat_list_next(ph, users))
                    if(!xchat_nickcmp(ph, xchat_list_str(ph, list, "nick"),word[3]))
                      sprintf(Buffer, "%s%s, ", Buffer, xchat_get_info(ph, "channel"));
                  xchat_list_free(ph, users);
                }
              }
          xchat_printf(ph, "Channels shared with %s: %s\n", word[3], Buffer);
          xchat_list_free(ph, list);
       }
     }
   }
*/
   if(!strcasecmp(word[2],"chancolorset")) { // set all channels to one color
     WasValid = 1;
     xchat_list *list = xchat_list_get(ph, "channels");
     if(list) {
        while(xchat_list_next(ph, list)) {
          if( xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context")) )
            xchat_commandf(ph, "gui color %s", (word[3]!=NULL ? word[3]: "0"));
        }
        xchat_list_free(ph, list);
     }
   }

   if(!strcasecmp(word[2],"chancolorset2")) { // for one server only
     WasValid = 1;
     const char *Server = xchat_get_info(ph, "server");
     if(Server != NULL) {
       xchat_list *list = xchat_list_get(ph, "channels");
       if(list) {
          while(xchat_list_next(ph, list)) {
            if(!strcasecmp(xchat_list_str(ph, list, "server"), Server)) {
              if( xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context")) )
                xchat_commandf(ph, "gui color %s", (word[3]!=NULL ? word[3]: "0"));
            }
          }
          xchat_list_free(ph, list);
       }
     }
   }

   if(!WasValid)
     xchat_printf(ph, "\"%s\" doesn't seem to be a Sparkles command\n", word[2]);
   return XCHAT_EAT_ALL;
}

static int invite_cb(char *word[], void *userdata) {
   if(JoinOnInvite > 0)
     if(strcasecmp(word[1],"#dontjoinitsatrap"))
       xchat_commandf(ph,"JOIN %s",word[1]);
   if(EatInvites > 0)
     return XCHAT_EAT_ALL;
   return XCHAT_EAT_NONE;
}

static int userquit_cb(char *word[], void *userdata) {
  if(AutoReclaimNick) {
    if(MyNickCmp(word[1], GhostReclaimNick)) {
      xchat_commandf(ph, "nick %s", GhostReclaimNick);
      AutoReclaimNick = 0;
    }
  }
  return XCHAT_EAT_NONE;
}

static int younick_cb(char *word[], void *userdata) {
  if(!DisableAutoNickDeblue)
    xchat_commandf(ph,"spark spawnquiet 1 s spark chancolorset2 0");
  return XCHAT_EAT_NONE;
}

static int youkick_cb(char *word[], void *userdata) {
   if(RejoinKick > 0)
     xchat_commandf(ph,"JOIN %s",word[2]);
   return XCHAT_EAT_NONE;
}

static int getnotice_cb(char *word[], void *userdata) {
/*
   if(MoveServicesToServer) { // still not fixed, oh wellll
     int Id;
     xchat_get_prefs(ph, "id", NULL, &Id);
     xchat_list *list = xchat_list_get(ph, "channels");
     if(list) {
       while(xchat_list_next(ph, list))
         if(xchat_list_int(ph, list, "type")==1 && xchat_list_int(ph, list, "id")==Id)
           xchat_set_context(ph,(xchat_context *)xchat_list_str(ph, list, "context"));
       xchat_list_free(ph, list);
     }
   }
*/

   if(MyNickCmp(word[1], "NickServ")) {
     if(!DisableAutoIdent)
       if(NULL != strstr(word[2],"is registered and protected.") || NULL != strstr(word[2],"Please choose a different nickname"))
         xchat_commandf(ph, "spark ident");
     if(AutoReclaimNick)
       if((NULL != strstr(word[2],"isn't currently in use")) ||
          (NULL != strstr(word[2],"ghost")) ||
          (NULL != strstr(word[2],"Ghost")) ||
          (NULL != strstr(word[2],"killed"))) {
         xchat_commandf(ph, "nick %s", GhostReclaimNick);
         xchat_commandf(ph,"spark spawnquiet 1 s nick %s", GhostReclaimNick);
         AutoReclaimNick = 0;
       }
   }
   return XCHAT_EAT_NONE;
}
static int nickclash_cb(char *word[], void *userdata) {
  if(((unsigned)time(NULL)) < (GhostDelayTime+9))
    return XCHAT_EAT_NONE;
  if(!DisableAutoGhost) {
    xchat_commandf(ph,"spark spawnquiet 2.2 s spark ghost2 %s", word[1]);
  }
  return XCHAT_EAT_NONE;
}

static int ctcp_cb(char *word[], void *userdata) {
  if(DisableSparklesCTCP)
    return XCHAT_EAT_NONE;
  if(!strcasecmp(word[1],"SPARKLES"))
    xchat_commandf(ph,"nctcp %s %s %s", word[2], PNAME, PVERSION);
  return XCHAT_EAT_NONE;
}

static int EatHighlightSay_cb(char *word[], void *userdata) {
  if(!EatHighlights)
    return XCHAT_EAT_NONE;
  xchat_emit_print(ph, "Channel Message", word[1], word[2], word[3], word[4], NULL);
  return XCHAT_EAT_ALL;
}

static int EatHighlightAct_cb(char *word[], void *userdata) {
  if(!EatHighlights)
    return XCHAT_EAT_NONE;
  xchat_emit_print(ph, "Channel Action", word[1], word[2], word[3], word[4], NULL);
  return XCHAT_EAT_ALL;
}

static int channelmessage_cb(char *word[], void *userdata) {
   // 1-nick, 2-text
   NeedSpaceBetweenX5Font = 0;
   int PrefixLen = strlen(SparkEncryptPrefix);

   int i = 0;
   if(!DisablePrettyJanus) {
     for(i=0;JanusLinkbots[i]!=NULL&&strcmp("",JanusLinkbots[i])&&i<4;i++)
       if(MyNickCmp(word[1], JanusLinkbots[i])) {
         if(word[2][0] == '<') {
           char Nick[80];
           char *Ends = strchr(word[2],'>');
           if(!Ends)
             break;
           char *Poke = Nick, *Peek = word[2]+1;
           while(*Peek != '>') {
             *(Poke++) = *(Peek++);
             if(Poke > (Nick+sizeof(Nick)))
               return XCHAT_EAT_NONE;
           }
           *Poke = 0;
           if(!Ends[1]) return XCHAT_EAT_NONE;
           if(!Ends[2]) return XCHAT_EAT_NONE;
           Ends+=2;

           xchat_commandf(ph, "recv :%s!sparkles@sparkles PRIVMSG %s :%s", Nick, xchat_get_info(ph, "channel"), Ends);
           return XCHAT_EAT_ALL;
         }
         if(word[2][0] == '*') {
           char Nick[80];
           char *Peek = word[2]+1;
           if(*Peek == ' ') Peek++;
           char *Poke = Nick;
           while(*Peek && (*Peek != ' ')) {
             *(Poke++) = *(Peek++);
           }
           *Poke = 0;
           if(!*Peek) return XCHAT_EAT_NONE;
           if(!Peek[1]) return XCHAT_EAT_NONE;
           if(!Peek[2]) return XCHAT_EAT_NONE;

           xchat_commandf(ph, "recv :%s!sparkles@sparkles PRIVMSG %s :\1ACTION %s\1", Nick, xchat_get_info(ph, "channel"), Peek+1);
           return XCHAT_EAT_ALL; 
         }
         break;
       }
   }   

   if((strlen(word[2]) > (PrefixLen + 7)) && !memcmp(word[2], SparkEncryptPrefix, strlen(SparkEncryptPrefix) )) {
     char *Peek = word[2]+PrefixLen;
     // \c__\c__\p

     if(Peek != NULL) {
       char Key1 = strtol(Peek+1,NULL,10) | (strtol(Peek+4,NULL,10)<<4);
       char Buffer[768]="";
       char *Poke = Buffer;

       Peek = strchr(Peek, 15); // look for the "normal formatting" code on the end
       if(Peek == NULL)
         return XCHAT_EAT_NONE;

       unsigned char kA = (Key1 & 192) >> 6;
       unsigned char kC = (Key1 & 15);
       while(*Peek) {
         char c = *(Peek++);
         char nyb = c &15;
         c&=~0x0f;
         if(c>=0x20) {
           switch(kA) {
             case 0: nyb -=kC; nyb ^=15; break;
             case 1: nyb -=kC; break;
             case 2: nyb +=kC; nyb ^=15; break;
             case 3: nyb +=kC; break;
           }
         }
         c|=(nyb&15);
         *(Poke++) = c;
       }
       *Poke = 0;

//       xchat_printf(ph, "(SE1)%s\xf\t%s\n", word[1], Buffer);
       xchat_commandf(ph, "recv :SE1/%s!sparkles@sparkles PRIVMSG %s :%s", word[1], xchat_get_info(ph, "channel"), Buffer); 
     }
     return XCHAT_EAT_ALL;
   }
   return XCHAT_EAT_NONE;
}

void INIConfigHandler(const char *Group, const char *Item, const char *Value) {
//  printf("[%s] %s = %s\n", Group, Item, Value);
  int i, *Int;
  char *String;
  for(i=0;ConfigOptions[i].Group!=NULL;i++)
    if(!strcasecmp(ConfigOptions[i].Group, Group)&&!strcasecmp(ConfigOptions[i].Item, Item)) {
      switch(ConfigOptions[i].Type) {
        case CONFIG_INTEGER:
          Int = ConfigOptions[i].Data;
          if(isdigit(*Value))
            *Int = strtol(Value, NULL, 10);
          else
            xchat_printf(ph, "Item \"[%s] %s\" requires a numeric value\n", Group, Item);
          return;
        case CONFIG_BOOLEAN:
          Int = ConfigOptions[i].Data;
          if(!strcasecmp(Value, "on") || !strcasecmp(Value, "yes"))
            *Int = 1;
          else if(!strcasecmp(Value, "off") || !strcasecmp(Value, "no"))
            *Int = 0;
          else
            xchat_printf(ph, "Item \"[%s] %s\" requires a value of on/yes or off/no (You put %s)\n", Group, Item, Value);
          return;
        case CONFIG_STRING:
          String = ConfigOptions[i].Data;
          if(strlen(Value)<ConfigOptions[i].Len)
            strcpy(String, Value);
          else
            xchat_printf(ph, "Item \"[%s] %s\" requires a smaller string\n", Group, Item);            
          return;
      }
      break;
    }
  xchat_printf(ph, "Config item \"[%s] %s\" not valid\n", Group, Item);
}

int ParseINI(FILE *File, void (*Handler)(const char *Group, const char *Item, const char *Value)) {
  char Group[512]="", *Item, *Value, Line[512]="", c, *Poke = NULL;
  if(File == NULL)
    return 0;
  xchat_printf(ph, "Sparkles config file found\n");
  int i;
  while(!feof(File)) {
    for(i=0,c=1;;i++) {
      c = fgetc(File);
      if(c=='\r'||c=='\n') {
        Line[i]=0;
        break;
      }
      Line[i] = c;
    }
    while(c=='\r'||c=='\n')
      c = fgetc(File);
    fseek(File, -1 , SEEK_CUR);
    if(!*Line)
      break;
    else if(*Line == ';'); // comment
    else if(*Line == '[') { // group
      Poke = strchr(Line, ']');
      if(Poke) *Poke = 0;
      strcpy(Group, Line+1);
    } else { // item
      Poke = strchr(Line, '=');
      if(Poke) {
        *Poke = 0;
        Item = Line;
        Value = Poke+1;
        Handler(Group, Item, Value);
      }
    }
  }
  fclose(File);
  return 1;
}

void xchat_plugin_get_info(char **name, char **desc, char **version, void **reserved) {
   *name = PNAME;
   *desc = PDESC;
   *version = PVERSION;
}
int xchat_plugin_deinit() {
   xchat_commandf(ph,"spark spawnquiet clear"); // no memory leaks please
   xchat_commandf(ph,"MENU -p4 DEL Sparkles");
   if(CustomYiffBuffer)
     free(CustomYiffBuffer);
   xchat_printf(ph,"Sparkles unloaded");
   return 1;
}

int xchat_plugin_init(xchat_plugin *plugin_handle,
                      char **plugin_name,
                      char **plugin_desc,
                      char **plugin_version,
                      char *arg) {
   /* we need to save this for use with any xchat_* functions */
   ph = plugin_handle;
   #if USE_SPARKLES_USER
     sprintf(SparklesUser, "%c01,09=Sparkles=%c\t", 3,15);
   #elseif
     strcpy(SparklesUser,"");
   #endif
   xchat_commandf(ph,"MENU -p4 ADD Sparkles");
   xchat_commandf(ph,"MENU ADD \"Sparkles/Settings\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/Auto-rejoin\" \"spark rejoin on\" \"spark rejoin off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/Join on invite\" \"spark invitejoin on\" \"spark invitejoin off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/No auto-ident\" \"spark autoident on\" \"spark autoident off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/No auto-nick-color-fix\" \"spark autonickdeblue on\" \"spark autonickdeblue off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/No +J auto-rejoin fix\" \"spark plusjfix on\" \"spark plusjfix off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/Switch gui focus on activity\" \"spark activity2focus on\" \"spark activity2focus off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/No autoghost\" \"spark autoghost on\" \"spark autoghost off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/No pesterchum assist\" \"spark pesterchum on\" \"spark pesterchum off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/No pretty janus\" \"spark prettyjanus on\" \"spark prettyjanus off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/Disable highlights\" \"spark set Automatic DisableHighlights on\" \"spark set Automatic DisableHighlights off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/Character counter\" \"spark set Automatic CharCounter on\" \"spark set Automatic CharCounter off\"");
   xchat_commandf(ph,"MENU -t0 ADD \"Sparkles/Settings/PM Alerts\" \"spark set Automatic PMAlerts on\" \"spark set Automatic PMAlerts off\"");

   xchat_commandf(ph,"MENU ADD \"Sparkles/Cram\"");
   xchat_commandf(ph,"MENU ADD \"Sparkles/Cram/Slash\" \"spark cram\"");
   xchat_commandf(ph,"MENU ADD \"Sparkles/Cram/No slash\" \"spark cram noslash\"");

   #ifdef HEXCHAT_PLUGIN_H
     xchat_commandf(ph,"MENU ADD \"Sparkles/HexChat\"");
     xchat_commandf(ph,"MENU ADD \"Sparkles/HexChat/Disable beeps\" \"set input_filter_beep on\"");
     xchat_commandf(ph,"MENU ADD \"Sparkles/HexChat/Enable beeps\" \"set input_filter_beep off\"");
   #else
     xchat_commandf(ph,"MENU ADD \"Sparkles/XChat\"");
     xchat_commandf(ph,"MENU ADD \"Sparkles/XChat/Disable beeps\" \"set input_filter_beep on\"");
     xchat_commandf(ph,"MENU ADD \"Sparkles/XChat/Enable beeps\" \"set input_filter_beep off\"");
   #endif

   xchat_commandf(ph,"MENU ADD \"Sparkles/Reset tab colors\" \"spark chancolorset 0\"");
   xchat_commandf(ph,"MENU ADD \"Sparkles/Remove sayhook\" \"spark sayhook off\"");
   xchat_commandf(ph,"MENU ADD \"Sparkles/Remove mehook\" \"spark mehook off\"");
   xchat_commandf(ph,"MENU ADD \"Sparkles/Save preferences\" \"spark saveprefs\"");   
   xchat_commandf(ph,"MENU ADD \"Sparkles/Sparkles Help\" \"spark help\"");

   /* tell xchat our info */
   *plugin_name = PNAME;   *plugin_desc = PDESC;   *plugin_version = PVERSION;

   /* make hooks */
   xchat_hook_command(ph, "spark", XCHAT_PRI_NORM, Spark_cb, "For info: /spark help", 0);

   memset(OnEventInfos, 0, sizeof(OnEventInfos));
   #if ENABLE_NSFW_CONTENT
     xchat_hook_command(ph, "yiff", XCHAT_PRI_NORM, yiff_cb, "Usage: /yiff [nick]", 0);
   #endif
   SayHook = xchat_hook_command(ph, "", XCHAT_PRI_NORM, TrapNormalPost_cb, NULL, 0);
   MeHook = xchat_hook_command(ph, "me", XCHAT_PRI_NORM, TrapActionPost_cb, NULL, 0);

   xchat_hook_print(ph, "Nick Clash",             XCHAT_PRI_NORM, nickclash_cb, NULL);
   xchat_hook_print(ph, "Channel Message",       XCHAT_PRI_NORM, channelmessage_cb, (int)0);
   xchat_hook_print(ph, "Channel Message",        XCHAT_PRI_LOW, Activity2Focus_cb, (int)0);
   xchat_hook_print(ph, "Channel Msg Hilight",    XCHAT_PRI_LOW, Activity2Focus_cb, (int)0);
   xchat_hook_print(ph, "Channel Action",         XCHAT_PRI_LOW, Activity2Focus_cb, (int)0);
   xchat_hook_print(ph, "Channel Action Hilight", XCHAT_PRI_LOW, Activity2Focus_cb, (int)0);

   xchat_hook_print(ph, "Channel Msg Hilight",    XCHAT_PRI_HIGH, EatHighlightSay_cb, (int)0);
   xchat_hook_print(ph, "Channel Action Hilight", XCHAT_PRI_HIGH, EatHighlightAct_cb, (int)0);

   xchat_hook_print(ph, "Notice", XCHAT_PRI_NORM, getnotice_cb, 0);
   xchat_hook_print(ph, "Quit", XCHAT_PRI_NORM, userquit_cb, 0);
   xchat_hook_print(ph, "You Join", XCHAT_PRI_NORM, WhatNetwork_cb,0);
   xchat_hook_print(ph, "Invited", XCHAT_PRI_LOW, invite_cb, 0);
   xchat_hook_print(ph, "CTCP Generic", XCHAT_PRI_LOW, ctcp_cb, 0);
   xchat_hook_print(ph, "CTCP Generic to Channel", XCHAT_PRI_LOW, ctcp_cb, 0);

   xchat_hook_print(ph, "You Kicked", XCHAT_PRI_LOW, youkick_cb, 0);
   xchat_hook_print(ph, "Your Nick Changing", XCHAT_PRI_NORM, younick_cb, 0);
   xchat_hook_server(ph, "RAW LINE", XCHAT_PRI_NORM, RawServer_cb, NULL);

   xchat_hook_timer(ph, 1000, timer_cb, NULL);
   xchat_hook_print(ph, "Key Press",    XCHAT_PRI_NORM, charcounter_cb, NULL); 
   xchat_hook_print(ph, "Your Message", XCHAT_PRI_NORM, charcounter_cb, NULL); 
   xchat_hook_print(ph, "Your Action",  XCHAT_PRI_NORM, charcounter_cb, NULL); 
   xchat_hook_print(ph, "Focus Tab",    XCHAT_PRI_NORM, charcounter_cb, NULL); 

   xchat_hook_print(ph, "Private Action",  XCHAT_PRI_NORM, privatemessage_cb, NULL); 
   xchat_hook_print(ph, "Private Message", XCHAT_PRI_NORM, privatemessage_cb, NULL); 
   xchat_hook_print(ph, "Private Action to Dialog",  XCHAT_PRI_NORM, privatemessage_cb, NULL); 
   xchat_hook_print(ph, "Private Message to Dialog", XCHAT_PRI_NORM, privatemessage_cb, NULL); 
   srand((unsigned)time(NULL));
   GhostDelayTime = (unsigned)time(NULL);

   sprintf(ConfigFilePath, "%s/Sparkles/sparkles.ini", xchat_get_info(ph, "xchatdirfs"));
   ParseINI(fopen(ConfigFilePath,"rb"), INIConfigHandler);

   FILE *Script;
   char Buffer[512];

   sprintf(Buffer, "%s/Sparkles/autoexec.txt", xchat_get_info(ph, "xchatdirfs"));
   int i,j,k;
   Script = fopen(Buffer,"r");
   if(Script != NULL) {
     xchat_print(ph, "Auto-exec script was found");
     for(k=0;!k;i=0) {
       i=0;
       while(1) {
         j = fgetc(Script);
         Buffer[i++]=j;
         if(j == '\n' || j==EOF) {
           Buffer[i-1] = 0;
           if(j==EOF) k=1;
             break;
         }
       }
       xchat_commandf(ph, "%s", Buffer);
     }
     fclose(Script);
   }

   xchat_printf(ph, "Sparkles version %s was loaded successfully \n",PVERSION);
   return 1;
}

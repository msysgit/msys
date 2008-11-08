
#include "multiuser.hh"


int GetAccountTypeHelper()
{
  int group = -1;
  HANDLE  hToken = NULL;
  struct group
  {
    DWORD auth_id;
    int type;
  };

  struct group groups[] = 
  {
    {DOMAIN_ALIAS_RID_USERS, MU_USER},
    // every user belongs to the users group, hence users come before guests
    {DOMAIN_ALIAS_RID_GUESTS, MU_GUEST},
    {DOMAIN_ALIAS_RID_POWER_USERS, MU_POWER},
    {DOMAIN_ALIAS_RID_ADMINS, MU_ADMIN}
  };

  if (GetVersion() & 0x80000000) // Not NT
  {
    return MU_ADMIN;
  }

  // First we must open a handle to the access token for this thread.
  if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken) ||
    OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
  {
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = {SECURITY_NT_AUTHORITY};
    TOKEN_GROUPS  *ptg          = NULL;
    BOOL       ValidTokenGroups = FALSE;
    DWORD      cbTokenGroups;
    DWORD      i, j;
    
    typedef BOOL (WINAPI*CHECKTOKENMEMBERSHIP)(HANDLE TokenHandle,PSID SidToCheck,PBOOL IsMember);
    CHECKTOKENMEMBERSHIP _CheckTokenMembership=NULL;
    // GetUserName is in advapi32.dll so we can avoid Load/Freelibrary
    _CheckTokenMembership=
      (CHECKTOKENMEMBERSHIP) GetProcAddress(
        GetModuleHandle("ADVAPI32"), "CheckTokenMembership");
    
    // Use "old school" membership check?
    if (_CheckTokenMembership == NULL)
    {
      // We must query the size of the group information associated with
      // the token. Note that we expect a FALSE result from GetTokenInformation
      // because we've given it a NULL buffer. On exit cbTokenGroups will tell
      // the size of the group information.
      if (!GetTokenInformation(hToken, TokenGroups, NULL, 0, &cbTokenGroups) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
        // Allocate buffer and ask for the group information again.
        // This may fail if an administrator has added this account
        // to an additional group between our first call to
        // GetTokenInformation and this one.
        if ((ptg = GlobalAlloc(GPTR, cbTokenGroups)) &&
          GetTokenInformation(hToken, TokenGroups, ptg, cbTokenGroups, &cbTokenGroups))
        {
          ValidTokenGroups=TRUE;
        }
      }
    }
    
    if (ValidTokenGroups || _CheckTokenMembership)
    {
      PSID psid;
      for (i = 0; i < sizeof(groups)/sizeof(struct group); i++)
      {
        // Create a SID for the local group and then check if it exists in our token
        if (AllocateAndInitializeSid(
          &SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
          groups[i].auth_id, 0, 0, 0, 0, 0, 0,&psid))
        {
          BOOL IsMember = FALSE;
          if (_CheckTokenMembership)
          {
            _CheckTokenMembership(0, psid, &IsMember);
          }
          else if (ValidTokenGroups)
          {
            for (j = 0; j < ptg->GroupCount; j++)
            {
              if (EqualSid(ptg->Groups[j].Sid, psid))
              {
                IsMember = TRUE;
              }
            }
          }
          
          if (IsMember) group=groups[i].type;
          FreeSid(psid);
        }
      }
    }

    if (ptg)
      GlobalFree(ptg);
    CloseHandle(hToken);

    return group;
  }

  return -1;
}

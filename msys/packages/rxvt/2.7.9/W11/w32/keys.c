static int keyTrans[256];
static int kt_init =0;

static void 
init_keytrans() {
  memset(keyTrans,0,sizeof(keyTrans));
  keyTrans[VK_CANCEL]= XK_Cancel;
  keyTrans[VK_CLEAR]=  XK_Clear;
  keyTrans[VK_PAUSE]=  XK_Pause;
  keyTrans[VK_PRIOR]=  XK_Prior;
  keyTrans[VK_NEXT]=   XK_Next;
  keyTrans[VK_END]=    XK_End;
  keyTrans[VK_HOME]=   XK_Home;
  keyTrans[VK_LEFT]=   XK_Left;
  keyTrans[VK_UP]=     XK_Up;
  keyTrans[VK_RIGHT]=  XK_Right;
  keyTrans[VK_DOWN]=   XK_Down;
  keyTrans[VK_SELECT]= XK_Select;
  keyTrans[VK_PRINT]=  XK_Print;
  keyTrans[VK_EXECUTE]=XK_Execute;
  keyTrans[VK_INSERT]= XK_Insert;
  keyTrans[VK_DELETE]= XK_Delete;
  keyTrans[VK_HELP]=   XK_Help;
  keyTrans[VK_NUMLOCK]=XK_Num_Lock;
  keyTrans[VK_SCROLL]= XK_Scroll_Lock;
  keyTrans[VK_BACK]=   XK_BackSpace;
  keyTrans[VK_F1]=     XK_F1;
  keyTrans[VK_F2]=     XK_F2;
  keyTrans[VK_F3]=     XK_F3;
  keyTrans[VK_F4]=     XK_F4;
  keyTrans[VK_F5]=     XK_F5;
  keyTrans[VK_F6]=     XK_F6;
  keyTrans[VK_F7]=     XK_F7;
  keyTrans[VK_F8]=     XK_F8;
  keyTrans[VK_F9]=     XK_F9;
  keyTrans[VK_F10]=    XK_F10;
  keyTrans[VK_F11]=    XK_F11;
  keyTrans[VK_F12]=    XK_F12;
  keyTrans[VK_ADD]=    XK_KP_Add;
  keyTrans[VK_SUBTRACT]=XK_KP_Subtract;
  init = 1;
}

int 
NT_translate_key(int in) {
  if (!kt_init) init_keytrans();
  return keyTrans[in & 0xff];
}

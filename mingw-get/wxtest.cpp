
#include <wx/app.h>
#include <wx/dialog.h>

extern "C" int ShowMainWnd2
 (HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpCmdLine,
  int nShowCmd)
{
	return ::wxEntry(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

class TestDialog : public wxDialog
{
public:
	TestDialog()
	 : wxDialog(0, -1, wxString("Hello"))
	{
	}

private:
	void OnClose(wxCloseEvent&)
	{
		Destroy();
	}

	DECLARE_EVENT_TABLE() //macro
};

BEGIN_EVENT_TABLE(TestDialog, wxDialog)
	EVT_CLOSE(TestDialog::OnClose)
END_EVENT_TABLE()


class MyApp : public wxApp
{
    virtual bool OnInit()
    {
    	TestDialog* dlg = new TestDialog;
    	dlg->Show();
    	return true;
    }
};

IMPLEMENT_APP_NO_MAIN(MyApp) //macro

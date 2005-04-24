; This sub-script defines any custom wizard pages
[Code]
Var
  VirusPage: TWizardPage;
  VirusURL: TLabel;
  VirusTextBox: TLabel;

Procedure VirusLinkOnClick(Sender: TObject);
Var
  ErrorCode: Integer;
Begin
  ShellExec('open', ExpandConstant('{cm:page_viruswarning_destination}'), '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
End;

Procedure InitializeWizard();
Begin
  VirusPage := CreateCustomPage
  ( wpInstalling,
    ExpandConstant('{cm:page_viruswarning_title}'),
    ExpandConstant('{cm:page_viruswarning_subtitle}')
  );

  VirusTextBox := TLabel.Create(VirusPage);
  VirusTextBox.Parent := VirusPage.Surface;
  VirusTextBox.Top := ScaleY(VirusTextBox.Font.Size);
  VirusTextBox.Caption := ExpandConstant('{cm:page_viruswarning_text}');
  VirusTextBox.WordWrap := True;
  VirusTextBox.Width := VirusPage.SurfaceWidth;
  VirusTextBox.Font.Size:= 10;

  VirusURL := TLabel.Create(VirusPage)
  VirusURL.Parent := VirusPage.Surface;
  VirusURL.Top := VirusTextBox.Height + VirusTextBox.Top + ScaleY(VirusTextBox.Font.Size);
  VirusURL.Caption := ExpandConstant('{cm:page_viruswarning_link}');
  VirusURL.Width := VirusPage.SurfaceWidth;
  VirusURL.Font.Size := VirusTextBox.Font.Size;
  VirusURL.Cursor := crHand;
  VirusURL.OnClick := @VirusLinkOnClick;
  { Alter Font *after* setting Parent so the correct defaults are inherited first }
  VirusURL.Font.Style := VirusURL.Font.Style + [fsUnderline];
  VirusURL.Font.Color := clBlue;
  

End;


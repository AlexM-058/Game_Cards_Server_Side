object Form4: TForm4
  Left = 0
  Top = 0
  Caption = 'Form4'
  ClientHeight = 441
  ClientWidth = 624
  Color = clHotLight
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object Label1: TLabel
    Left = 135
    Top = 40
    Width = 38
    Height = 15
    Caption = 'Player1'
  end
  object Label2: TLabel
    Left = 135
    Top = 69
    Width = 38
    Height = 15
    Caption = 'Player2'
  end
  object Label3: TLabel
    Left = 135
    Top = 99
    Width = 38
    Height = 15
    Caption = 'Player3'
  end
  object Label4: TLabel
    Left = 135
    Top = 128
    Width = 38
    Height = 15
    Caption = 'Player4'
  end
  object CountDown: TLabel
    Left = 480
    Top = 168
    Width = 64
    Height = 15
    Caption = 'CountDown'
  end
  object Play1: TEdit
    Left = 8
    Top = 32
    Width = 121
    Height = 23
    TabOrder = 0
    Text = 'Play1'
  end
  object Play2: TEdit
    Left = 8
    Top = 61
    Width = 121
    Height = 23
    TabOrder = 1
    Text = 'Play2'
  end
  object Play3: TEdit
    Left = 8
    Top = 90
    Width = 121
    Height = 23
    TabOrder = 2
    Text = 'Play3'
  end
  object Play4: TEdit
    Left = 8
    Top = 120
    Width = 121
    Height = 23
    TabOrder = 3
    Text = 'Play4'
  end
  object Reset: TButton
    Left = 296
    Top = 368
    Width = 75
    Height = 25
    Caption = 'Reset'
    TabOrder = 4
    OnClick = ResetClick
  end
  object IdTCPServer1: TIdTCPServer
    Active = True
    Bindings = <
      item
        IP = '0.0.0.0'
        Port = 6770
      end>
    DefaultPort = 6770
    OnConnect = IdTCPServer1Connect
    OnExecute = IdTCPServer1Execute
    Left = 304
    Top = 224
  end
  object Timer1: TTimer
    Interval = 200
    OnTimer = Timer1Timer
    Left = 456
    Top = 360
  end
end

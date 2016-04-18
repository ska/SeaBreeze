VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   5700
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   10560
   LinkTopic       =   "Form1"
   ScaleHeight     =   5700
   ScaleWidth      =   10560
   StartUpPosition =   3  '´°¿ÚÈ±Ê¡
   Begin VB.TextBox Text1 
      Height          =   375
      Left            =   5520
      TabIndex        =   2
      Text            =   "100"
      Top             =   240
      Width           =   975
   End
   Begin VB.ListBox List 
      Height          =   4020
      ItemData        =   "sample.frx":0000
      Left            =   600
      List            =   "sample.frx":0002
      TabIndex        =   1
      Top             =   1080
      Width           =   9375
   End
   Begin VB.CommandButton Run 
      Caption         =   "Run"
      Height          =   495
      Left            =   360
      TabIndex        =   0
      Top             =   240
      Width           =   1215
   End
   Begin VB.Label Label1 
      Caption         =   "ms"
      Height          =   375
      Left            =   6720
      TabIndex        =   4
      Top             =   240
      Width           =   495
   End
   Begin VB.Label IntegrationTime 
      Caption         =   "Integration Time:"
      BeginProperty Font 
         Name            =   "ËÎÌå"
         Size            =   10.5
         Charset         =   134
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   2760
      TabIndex        =   3
      Top             =   240
      Width           =   2055
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim error_code As Long
Dim result As Integer
Dim wavelengths() As Double
Dim spectrum() As Double
Dim spectrum_length As Integer
Dim error_string(20) As Byte
Dim string_length As Long


Private Sub Form_Load()
result = seabreeze_open_spectrometer(0, error_code)
If error_code <> 0 Then
    string_length = seabreeze_get_error_string(error_code, error_string(0), 20)
    List.AddItem (IntToString(error_string, string_length))
End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
result = seabreeze_close_spectrometer(0, error_code)
End Sub

Private Sub Run_Click()
Dim pixel As Integer
Dim indices(10) As Long
Dim epprom(18) As Double



If error_code = 0 Then
List.Clear
seabreeze_set_integration_time 0, error_code, CDbl(Text1.Text) * 1000

spectrum_length = seabreeze_get_formatted_spectrum_length(0, error_code)

ReDim wavelengths(spectrum_length)
ReDim spectrum(spectrum_length)
result = seabreeze_get_wavelengths(0, error_code, wavelengths(0), spectrum_length)

result = seabreeze_get_formatted_spectrum(0, error_code, spectrum(0), spectrum_length)

For pixel = 0 To spectrum_length - 1
List.AddItem ("pixel number: " & pixel & " wavelength is " & wavelengths(pixel) & " spectrum is " & spectrum(pixel))
Next

result = seabreeze_get_electric_dark_pixel_indices(0, error_code, indices(0), 10)

result = seabreeze_read_eeprom_slot(0, error_code, 0, epprom(0), 18)

result = seabreeze_read_GPIOBit(0, error_code, 1)
End If
End Sub

Private Function IntToString(str As Variant, length As Long) As String
Dim i As Integer
Dim t As String

For i = 0 To length
t = t & Chr(str(i))
Next
IntToString = t
End Function




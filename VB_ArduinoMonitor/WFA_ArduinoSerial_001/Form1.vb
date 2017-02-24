
Imports System
Imports System.IO.Ports

Public Class Form1

    Structure freq_component
        Dim Freq As UInt64
        Dim Value As UInt64
    End Structure

    ' Global variables
    Dim comPORT As String
    Dim receivedData As String = ""
    Dim connected As Boolean = False
    Dim count = 0
    Dim frequencies(128) As freq_component
    Dim volume_array(1000) As Integer
    Public Shared input_buffer As String

    ' When the program starts; make sure the timer is off (not really needed) and add the available COM ports to the COMport drop down list
    Private Sub Form1_Load(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles MyBase.Load
        Timer1.Enabled = False
        populateCOMport()
        frequencies(0).Freq = 0

        For index = 1 To 128
            frequencies(index).Freq = frequencies(index - 1).Freq + 150
        Next
    End Sub

    'The refresh button updates the COMport list
    Private Sub refreshCOM_BTN_Click(sender As Object, e As EventArgs) Handles refreshCOM_CB_BTN.Click
        If (comPORT <> "") Then
            MsgBox("Disconnect COM port first")
        Else
            populateCOMport()
        End If
    End Sub

    Private Sub populateCOMport()
        comPORT = ""
        ComPort_ComboBox.Items.Clear()
        For Each sp As String In My.Computer.Ports.SerialPortNames
            ComPort_ComboBox.Items.Add(sp)
        Next
    End Sub

    Private Sub comPort_ComboBox_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ComPort_ComboBox.SelectedIndexChanged
        If (ComPort_ComboBox.SelectedItem <> "") Then
            comPORT = ComPort_ComboBox.SelectedItem
        End If
    End Sub


    ' When the Connect button is clicked; if a COM port has been selected, connect and send out a HELLO message.
    ' Then wait for the Arduino to respond with its own HELLO.
    ' When the HELLO is received we are connected; change the button text to Disconnect.
    Private Sub connect_BTN_Click(sender As Object, e As EventArgs) Handles connect_BTN.Click
        comPORT = ComPort_ComboBox.SelectedItem
        If (connect_BTN.Text = "Connect") Then
            If (comPORT <> "") Then
                SerialPort1.Close()
                SerialPort1.PortName = comPORT
                SerialPort1.BaudRate = 9600
                SerialPort1.DataBits = 8
                SerialPort1.Parity = Parity.None
                SerialPort1.StopBits = StopBits.One
                SerialPort1.Handshake = Handshake.None
                SerialPort1.Encoding = System.Text.Encoding.Default
                SerialPort1.ReadTimeout = 10000

                Try
                    SerialPort1.Open()
                    'See if the Arduino is there
                    count = 0
                    connect_BTN.Text = "Connecting..."
                    connecting_Timer.Enabled = True
                Catch ex As UnauthorizedAccessException
                    MsgBox("Error: Serial port is already open in another program")
                End Try

            Else
                MsgBox("Select a COM port first")
            End If
        Else
            connect_BTN.Text = "Disconnect"
            'close the connection, reset the button and timer label
            Timer1.Enabled = False
            Timer_LBL.Text = "Timer: OFF"
            SerialPort1.Close()
            connected = False
            connect_BTN.Text = "Connect"
            populateCOMport()
        End If

    End Sub

    'The connecting_Timer waits for the Arduino to say HELLO.
    ' If HELLO is not received in 2 seconds display an error message.
    ' The connecting_Timer is only used for connecting
    Private Sub connecting_Timer_Tick(sender As Object, e As EventArgs) Handles connecting_Timer.Tick

        connecting_Timer.Enabled = False
        count = count + 1

        If (count <= 25) Then
            receivedData = receivedData & ReceiveSerialData()

            If (comPORT <> "") Then
                'if we get an HELLO from the Arduino then we are connected
                connected = True
                connect_BTN.Text = "Disconnect"
                Timer1.Enabled = True
                Timer_LBL.Text = "Timer: ON"
                receivedData = ReceiveSerialData()
                receivedData = ""

            Else
                'start the timer again and keep waiting for a signal from the Arduino
                connecting_Timer.Enabled = True
            End If


        Else
            'time out (8 * 250 = 2 seconds)
            RichTextBox1.Text &= vbCrLf & "ERROR" & vbCrLf & "Cannot connect" & vbCrLf
            connect_BTN.Text = "Connect"
            populateCOMport()
        End If



    End Sub

    'After a connection is made the main timer waits for data from the Arduino
    Public Sub Timer1_Tick(sender As Object, e As EventArgs) Handles Timer1.Tick
        Dim temp_val As String = 0
        Dim peak_freq_index As Integer = 0
        Dim peak_value As Integer
        Dim fundemental_freq As UInt64 = 0

        receivedData = ReceiveSerialData()
        RichTextBox1.Text &= receivedData

        If (receivedData = "[end]") Then

            'frequ 1 - not used
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            frequencies(0).Value = 0

            'frequ 2 - not used
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            frequencies(1).Value = 0

            'frequ 3
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar1.Value = temp_val
            frequencies(2).Value = temp_val

            'frequ 2
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar2.Value = temp_val
            frequencies(3).Value = temp_val

            'frequ 3
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar3.Value = temp_val
            frequencies(4).Value = temp_val

            'frequ 4
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar4.Value = temp_val
            frequencies(5).Value = temp_val

            'frequ 5
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar5.Value = temp_val
            frequencies(6).Value = temp_val

            'frequ 6
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar6.Value = temp_val
            frequencies(7).Value = temp_val

            'frequ 7
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar7.Value = temp_val
            frequencies(8).Value = temp_val

            'frequ 8
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar8.Value = temp_val
            frequencies(9).Value = temp_val

            'frequ 9
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar9.Value = temp_val
            frequencies(10).Value = temp_val

            'frequ 10
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar10.Value = temp_val
            frequencies(11).Value = temp_val

            'frequ 11
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar11.Value = temp_val
            frequencies(12).Value = temp_val

            'frequ 12
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar12.Value = temp_val
            frequencies(13).Value = temp_val

            'frequ 13
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar13.Value = temp_val
            frequencies(14).Value = temp_val

            'frequ 14
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar14.Value = temp_val
            frequencies(15).Value = temp_val

            'frequ 15
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar15.Value = temp_val
            frequencies(16).Value = temp_val

            'frequ 16
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar16.Value = temp_val
            frequencies(17).Value = temp_val

            'frequ 17
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar17.Value = temp_val
            frequencies(18).Value = temp_val

            'frequ 18
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar18.Value = temp_val
            frequencies(19).Value = temp_val

            'frequ 19
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar19.Value = temp_val
            frequencies(20).Value = temp_val

            'frequ 20
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar20.Value = temp_val
            frequencies(21).Value = temp_val

            'frequ 21
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar21.Value = temp_val
            frequencies(22).Value = temp_val

            'frequ 22
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar22.Value = temp_val
            frequencies(23).Value = temp_val

            'frequ 23
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar23.Value = temp_val
            frequencies(24).Value = temp_val

            'frequ 24
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar24.Value = temp_val
            frequencies(25).Value = temp_val

            'frequ 25
            temp_val = Microsoft.VisualBasic.Left(receivedData, InStr(receivedData, ","))
            receivedData = LTrim(Microsoft.VisualBasic.Right(receivedData, receivedData.Length - InStr(receivedData, ",")))
            TrackBar25.Value = temp_val
            frequencies(26).Value = temp_val

            For index = 0 To 127
                If (frequencies(index).Value > peak_value) Then
                    peak_value = frequencies(index).Value
                    peak_freq_index = index
                End If
            Next

            fundemental_freq = frequencies(peak_freq_index).Freq
            FundFreqTBox.Text = fundemental_freq
            RichTextBox1.Text &= receivedData

            TextBox2.Text = frequencies(peak_freq_index).Value

            Dim tmpStr As String
            tmpStr = Microsoft.VisualBasic.Left(receivedData, 25)
        End If
    End Sub

    Function ReceiveSerialData() As String
        Dim Incoming As String
        Dim start_pos As Integer = 0
        Dim end_pos As Integer = 0
        Dim length As Integer = 0
        Dim return_string As String

        'Try
        'Incoming = SerialPort1.ReadExisting()
        'If Incoming Is Nothing Then
        'Return "nothing" & vbCrLf
        ' Else
        'Return Incoming
        ' End If
        'Catch ex As TimeoutException
        'Return "Error: Serial Port read timed out."
        'End Try


        Try
            Incoming = SerialPort1.ReadExisting()

            input_buffer = input_buffer + Incoming

            If InStr(input_buffer, "[end]") Then
                If InStr(input_buffer, "[start]") Then
                    start_pos = InStr(input_buffer, "[start]")
                    end_pos = InStr(input_buffer, "[end]")

                    If (start_pos < end_pos) Then
                        input_buffer = Mid(input_buffer, InStr(input_buffer, "[start]"))

                        start_pos = InStr(input_buffer, "[start]")
                        end_pos = InStr(input_buffer, "[end]")
                        length = (end_pos + 5) - start_pos

                        return_string = Mid(input_buffer, start_pos + 9, length - 14)
                        input_buffer = Mid(input_buffer, length + 3)
                        Return return_string
                    Else
                        input_buffer = Mid(input_buffer, end_pos + 5)
                        Return Nothing
                    End If
                Else
                    input_buffer = Mid(input_buffer, InStr(input_buffer, "[end]"))
                    Return Nothing
                End If


            Else
                'Incoming = SerialPort1.ReadExisting()
                ' If Incoming Is Nothing Then
                'Return "nothing" & vbCrLf
                'Else
                Return Incoming
            End If
            ' End If


        Catch ex As TimeoutException
            Return "Error: Serial Port read timed out."
        End Try
        Return Nothing



    End Function

    'Send text input
    'Does not clear the contents
    Private Sub send_BTN_Click(sender As Object, e As EventArgs) Handles send_BTN.Click
        If (connected) Then
            Dim tmp As String
            tmp = send_TB.Text
            SerialPort1.Write(tmp)
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private img As Bitmap
    Private imgClone As Bitmap
    Private widthInterval As Integer
    'the distance from the left side of picturebox where x axis starts
    Private leftPad As Integer = 50
    'the distance from the down side of picturebox where x axis is
    Private downPad As Integer = 30
    'the distance from the up side of picturebox where y axis ends
    Private upPad As Integer = 50
    'the distance from the right side of picturebox where x axis ends
    Private rightPad As Integer = 80
    Private rn As New Random

    Private Sub Button19_Click(sender As Object, e As EventArgs) Handles Button19.Click
        If (connected) Then
            SerialPort1.Write("PM1")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button18_Click(sender As Object, e As EventArgs) Handles Button18.Click
        If (connected) Then
            SerialPort1.Write("PM0")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button17_Click(sender As Object, e As EventArgs) Handles Button17.Click
        If (connected) Then
            SerialPort1.Write("RESET")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        If (connected) Then
            SerialPort1.Write("MVM,1")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        If (connected) Then
            SerialPort1.Write("MVM,2")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button4_Click(sender As Object, e As EventArgs) Handles Button4.Click
        If (connected) Then
            SerialPort1.Write("MVM,3")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button5_Click(sender As Object, e As EventArgs) Handles Button5.Click
        If (connected) Then
            SerialPort1.Write("MVM,4")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button6_Click(sender As Object, e As EventArgs) Handles Button6.Click
        If (connected) Then
            SerialPort1.Write("MVM,5")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button11_Click(sender As Object, e As EventArgs) Handles Button11.Click
        If (connected) Then
            SerialPort1.Write("MCM,1")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button10_Click(sender As Object, e As EventArgs) Handles Button10.Click
        If (connected) Then
            SerialPort1.Write("MCM,2")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button9_Click(sender As Object, e As EventArgs) Handles Button9.Click
        If (connected) Then
            SerialPort1.Write("MCM,3")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button8_Click(sender As Object, e As EventArgs) Handles Button8.Click
        If (connected) Then
            SerialPort1.Write("MCM,4")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button7_Click(sender As Object, e As EventArgs) Handles Button7.Click
        If (connected) Then
            SerialPort1.Write("MCM,5")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button16_Click(sender As Object, e As EventArgs) Handles Button16.Click
        If (connected) Then
            SerialPort1.Write("MLM,1")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button15_Click(sender As Object, e As EventArgs) Handles Button15.Click
        If (connected) Then
            SerialPort1.Write("MLM,2")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button14_Click(sender As Object, e As EventArgs) Handles Button14.Click
        If (connected) Then
            SerialPort1.Write("MLM,3")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button13_Click(sender As Object, e As EventArgs) Handles Button13.Click
        If (connected) Then
            SerialPort1.Write("MLM,4")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button12_Click(sender As Object, e As EventArgs) Handles Button12.Click
        If (connected) Then
            SerialPort1.Write("MLM,5")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button21_Click(sender As Object, e As EventArgs) Handles Button21.Click
        If (connected) Then
            SerialPort1.Write("FFT1")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        If (connected) Then
            SerialPort1.Write("FFT2")
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    Private Sub GraphBtn_Click(sender As Object, e As EventArgs) Handles GraphBtn.Click
        If (connected) Then
            SerialPort1.Write("FFT3")
            Static count As Boolean = False
            Dim g As Graphics
            'number of values in x axis e.g 1, 2, 3, ... representing time
            Dim numX As Integer = 10
            'number of values in y axis representing KW/h
            Dim numY As Integer = 5
            Dim stringFormat As New StringFormat()
            'arreys to hold the text for both axies
            Dim arrayTextX(numX), arrayTextY(numY - 1) As String
            Dim i As Integer
            'the distance from the right side of picturebox where x axis stops
            Dim rightPad As Integer = 80
            Dim brush As Brush = New SolidBrush(Color.FromArgb(245, 255, 255))
            Dim pen As Pen = New Pen(Color.FromArgb(212, 212, 212), 1)
            Dim height, x, y As Integer

            Dim pntEnd As Point
            Static pnt As Point = New Point(-1, -1)
            Static staticX As Integer = -1

            'Run once
            If count = True Then
                Return
            End If

            count = True

            stringFormat.Alignment = StringAlignment.Center

            img = New Bitmap(PictureBox1.Width, PictureBox1.Height)
            imgClone = New Bitmap(PictureBox1.Width, PictureBox1.Height)

            g = Graphics.FromImage(img)
            g.SmoothingMode = Drawing2D.SmoothingMode.AntiAlias
            g.Clear(Color.White)

            'the distance in x axis between each value
            widthInterval = CInt((PictureBox1.Width - leftPad - rightPad) / (numX + 1))
            'the distance in y axis between each value
            height = CInt((PictureBox1.Height - upPad - downPad) / (numY + 1))

            'fill arrays with text
            For i = 0 To numX - 1
                arrayTextX(i) = (i + 1).ToString
            Next

            For i = 0 To numY - 1
                arrayTextY(i) = ((i + 1) * height).ToString
            Next

            'fill background of graph with color
            g.FillRectangle(brush, New Rectangle(leftPad, upPad, PictureBox1.Width - leftPad - rightPad + 1, _
                                                 PictureBox1.Height - downPad - upPad))
            'vertical lines
            x = leftPad
            For i = 0 To numX - 1
                x += widthInterval
                g.DrawLine(pen, x, PictureBox1.Height - downPad, x, upPad)
                g.DrawString(arrayTextX(i), New Font("Arial", 8), Brushes.Black, _
                             New Rectangle(x - 10, PictureBox1.Height - downPad + 3, 20, 20), stringFormat)
            Next

            'horizontal lines
            stringFormat.Alignment = StringAlignment.Far
            y = PictureBox1.Height - downPad
            For i = 0 To numY - 1
                y -= height
                g.DrawLine(pen, leftPad, y, PictureBox1.Width - rightPad, y)
                g.DrawString(arrayTextY(i), New Font("Arial", 8), Brushes.Black, _
                             New Rectangle(0, y - 6, leftPad - 5, 20), stringFormat)
            Next

            g.DrawString("KW/Hour", New Font("Arial", 8, FontStyle.Bold), Brushes.Black, _
                             New PointF(5, 5))
            g.DrawString("Time", New Font("Arial", 8, FontStyle.Bold), Brushes.Black, _
                             New PointF(PictureBox1.Width - 50, PictureBox1.Height - 20))

            'draws x axis
            g.DrawLine(Pens.Black, New Point(leftPad, PictureBox1.Height - downPad), _
                       New Point(PictureBox1.Width - rightPad, PictureBox1.Height - downPad))
            'draws y axis
            g.DrawLine(Pens.Black, New Point(leftPad, PictureBox1.Height - downPad), _
                       New Point(leftPad, upPad))


            pnt = New Point(leftPad + 30, PictureBox1.Height - 0 - downPad)
            pntEnd = New Point(leftPad + 30, PictureBox1.Height - 200 - downPad)
            g.DrawLine(Pens.Red, pnt, pntEnd)

            pnt = New Point(leftPad + 45, PictureBox1.Height - 0 - downPad)
            pntEnd = New Point(leftPad + 45, PictureBox1.Height - 200 - downPad)
            g.DrawLine(Pens.Red, pnt, pntEnd)

            pnt = New Point(leftPad + 60, PictureBox1.Height - 0 - downPad)
            pntEnd = New Point(leftPad + 60, PictureBox1.Height - 200 - downPad)
            g.DrawLine(Pens.Red, pnt, pntEnd)

            g.Dispose()

            PictureBox1.Image = img
            imgClone = CType(img.Clone, Bitmap)
        Else
            MsgBox("You must first connect to a COM port")
        End If
    End Sub

    'Clear any input
    Private Sub ClearAllBtn_Click(sender As Object, e As EventArgs) Handles clear_BTN.Click
        RichTextBox1.Text = ""
        RichTextBox1.Text = ""
        FundFreqTBox.Text = ""
        TextBox1.Text = ""
        TextBox6.Text = ""
        TextBox2.Text = ""
        send_TB.Text = ""
        PictureBox1.Image = Nothing
        TrackBar1.Value = 0
        TrackBar2.Value = 0
        TrackBar3.Value = 0
        TrackBar4.Value = 0
        TrackBar5.Value = 0
        TrackBar6.Value = 0
        TrackBar7.Value = 0
        TrackBar8.Value = 0
        TrackBar9.Value = 0
        TrackBar10.Value = 0
        TrackBar11.Value = 0
        TrackBar12.Value = 0
        TrackBar13.Value = 0
        TrackBar14.Value = 0
        TrackBar15.Value = 0
        TrackBar16.Value = 0
        TrackBar17.Value = 0
        TrackBar18.Value = 0
        TrackBar19.Value = 0
        TrackBar20.Value = 0
        TrackBar21.Value = 0
        TrackBar22.Value = 0
        TrackBar23.Value = 0
        TrackBar24.Value = 0
        TrackBar25.Value = 0
    End Sub

End Class

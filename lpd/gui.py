from udp_client import send_buf, send_buf2, N_LEDS, hsv
import wx

class MyFrame(wx.Frame):
    def __init__(self, parent, id, title):

        wx.Frame.__init__(self, parent, id, title, wx.DefaultPosition, (600, 250))
        panel = wx.Panel(self, -1)
        self.panel = panel

        vbox = wx.BoxSizer(wx.VERTICAL)
        hbox = wx.BoxSizer(wx.HORIZONTAL)


        self.slider1 = wx.Slider(panel, 0xa001, 100, 0, 255, wx.DefaultPosition, (450, -1),
                              wx.SL_AUTOTICKS | wx.SL_HORIZONTAL | wx.SL_LABELS)
        self.slider2 = wx.Slider(panel, 0xa002, 200, 0, 255, wx.DefaultPosition, (450, -1),
                              wx.SL_AUTOTICKS | wx.SL_HORIZONTAL | wx.SL_LABELS)
        self.slider3 = wx.Slider(panel, 0xa003, 200, 0, 255, wx.DefaultPosition, (450, -1),
                              wx.SL_AUTOTICKS | wx.SL_HORIZONTAL | wx.SL_LABELS)

        btn1 = wx.Button(panel, 8, 'Adjust')
        btn2 = wx.Button(panel, 9, 'Close')



        wx.EVT_SLIDER(self, 0xa001, self.OnSlide)
        wx.EVT_SLIDER(self, 0xa002, self.OnSlide)
        wx.EVT_SLIDER(self, 0xa003, self.OnSlide)

        wx.EVT_BUTTON(self, 8, self.OnAdjust)
        wx.EVT_BUTTON(self, 9, self.OnClose)
        vbox.Add(self.slider1, 1, wx.ALIGN_CENTRE)
        vbox.Add(self.slider2, 1, wx.ALIGN_CENTRE)
        vbox.Add(self.slider3, 1, wx.ALIGN_CENTRE)
#        hbox.Add(btn1, 1, wx.RIGHT, 10)
#        hbox.Add(btn2, 1)
        vbox.Add(hbox, 0, wx.ALIGN_CENTRE | wx.ALL, 20)
        panel.SetSizer(vbox)

    def OnAdjust(self, event):
        val = self.sld.GetValue()

        self.SetSize((val*2, val))
    def OnClose(self, event):
        self.Close()

    def OnSlide(self, event):
        r = self.slider1.GetValue()
        g = self.slider2.GetValue()
        b = self.slider3.GetValue()
        self.SetBackgroundColour(wx.Colour(r,g,b,0.5))
        #buf = [r,g,b] * N_LEDS
        buf = hsv(r,g,b) * N_LEDS
        send_buf(buf)

class MyApp(wx.App):
    def OnInit(self):
        frame = MyFrame(None, -1, 'slider.py')
        frame.Show(True)
        frame.Centre()
        return True

app = MyApp(0)
app.MainLoop()

#
## Copyright (c) 2018, Bradley A. Minch
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
##     1. Redistributions of source code must retain the above copyright
##        notice, this list of conditions and the following disclaimer.
##     2. Redistributions in binary form must reproduce the above copyright
##        notice, this list of conditions and the following disclaimer in the
##        documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
#

import tkinter as tk
import usbtest

class usbtestgui:

    def __init__(self):
        self.dev = usbtest.usbtest()
        if self.dev.dev >= 0:
            self.update_job = None
            self.root = tk.Tk()
            self.root.title('USB Test GUI')
            self.root.protocol('WM_DELETE_WINDOW', self.shut_down)
            fm = tk.Frame(self.root)
            tk.Button(fm, text = 'LED1', command = self.dev.toggle_led1).pack(side = tk.LEFT)
            tk.Button(fm, text = 'LED2', command = self.dev.toggle_led2).pack(side = tk.LEFT)
            tk.Button(fm, text = 'LED3', command = self.dev.toggle_led3).pack(side = tk.LEFT)
            fm.pack(side = tk.TOP)
            dutyslider = tk.Scale(self.root, from_ = 0, to = 100, orient = tk.HORIZONTAL, showvalue = tk.FALSE, command = self.set_duty_callback)
            dutyslider.set(25)
            dutyslider.pack(side = tk.TOP)
            self.sw1_status = tk.Label(self.root, text = 'SW1 is currently ?')
            self.sw1_status.pack(side = tk.TOP)
            self.sw2_status = tk.Label(self.root, text = 'SW2 is currently ?')
            self.sw2_status.pack(side = tk.TOP)
            self.sw3_status = tk.Label(self.root, text = 'SW3 is currently ?')
            self.sw3_status.pack(side = tk.TOP)
            self.a0_status = tk.Label(self.root, text = 'A0 is currently ????')
            self.a0_status.pack(side = tk.TOP)
            self.update_status()

    def set_duty_callback(self, value):
        self.dev.set_duty(float(value))

    def update_status(self):
        self.sw1_status.configure(text = 'SW1 is currently {!s}'.format(self.dev.read_sw1()))
        self.sw2_status.configure(text = 'SW2 is currently {!s}'.format(self.dev.read_sw2()))
        self.sw3_status.configure(text = 'SW3 is currently {!s}'.format(self.dev.read_sw3()))
        self.a0_status.configure(text = 'A0 is currently {:04d}'.format(self.dev.read_a0()))
        self.update_job = self.root.after(50, self.update_status)

    def shut_down(self):
        self.root.after_cancel(self.update_job)
        self.root.destroy()
        self.dev.close()

if __name__=='__main__':
    gui = usbtestgui()
    gui.root.mainloop()

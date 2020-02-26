import tkinter as tk
import modeselect
import csv

class modeselectgui:

    def __init__(self):
        self.dev = modeselect.modeselect()
        if self.dev.dev is not None:
            self.update_job = None
            self.root = tk.Tk()
            self.root.title('Mode Select GUI')
            self.root.protocol('WM_DELETE_WINDOW', self.shut_down)
            fm = tk.Frame(self.root)
            tk.Button(fm, text = 'Spring', command = self.dev.spring).pack(side = tk.LEFT)
            tk.Button(fm, text = 'Damper', command = self.dev.damper).pack(side = tk.LEFT)
            tk.Button(fm, text = 'Texture', command = self.dev.texture).pack(side = tk.LEFT)
            tk.Button(fm, text = 'Wall', command = self.dev.wall).pack(side = tk.LEFT)
            fm.pack(side = tk.TOP)
            self.update_status()

    def update_status(self):
        self.update_job = self.root.after(1, self.update_status)

    def shut_down(self):
        self.root.after_cancel(self.update_job)
        self.root.destroy()
        self.dev.close()

if __name__=='__main__':
    gui = modeselectgui()
    gui.root.mainloop()

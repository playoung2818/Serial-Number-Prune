import tkinter as tk
from tkinter import scrolledtext
import re

def extract_useful_number(sn: str) -> str:
    """
    Extracts the part after SN or SNO in a serial number string.
    Example: "Pcie-poe, SN0023783" => "0023783"
    """
    match = re.search(r"SN?([A-Za-z0-9]+)", sn)
    return match.group(1) if match else ""

def prune_serials():
    raw_input = input_text.get("1.0", tk.END)
    raw_list = re.split(r'[,\n;]+', raw_input)
    cleaned = [extract_useful_number(s.strip()) for s in raw_list]
    cleaned = sorted(set(filter(None, cleaned)))
    result_text.delete("1.0", tk.END)
    result_text.insert(tk.END, "\n".join(cleaned))

# Build GUI
window = tk.Tk()
window.title("Serial Number Pruner")
window.geometry("600x400")

tk.Label(window, text="Paste Serial Numbers:").pack()
input_text = scrolledtext.ScrolledText(window, height=8, wrap=tk.WORD)
input_text.pack(fill="both", padx=10, pady=5)

tk.Button(window, text="Prune and Sort", command=prune_serials).pack(pady=10)

tk.Label(window, text="Cleaned Serial Numbers:").pack()
result_text = scrolledtext.ScrolledText(window, height=8, wrap=tk.WORD)
result_text.pack(fill="both", padx=10, pady=5)

window.mainloop()

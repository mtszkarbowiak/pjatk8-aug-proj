import sys

def remove_eols(in_file):
    out_file = in_file + ".n"
    
    with open(in_file, 'r') as f:
        content = f.read()
    
    content = content.replace('\n', '').replace('\r', '')
    
    with open(out_file, 'w') as f2:
        f2.write(content)
    
    print('Done')
    

if __name__ == "__main__":
    remove_eols(sys.argv[1])

# Reducer: Simple Loadable Linux Kernel Module

Reducer is a LKM that registers a char device, and sums up all the numbers written to it.
This a very simple module, created to learn about char devices and Kernal Modules.

# Usage
Once the module is loaded, a character device file named `/dev/reducer` will be created(see Installation steps)
#### Write numbers 
```bash
echo 15 > /dev/reducer
echo 6 > /dev/reducer
```
#### Reading the Sum
```bash
cat /dev/reducer
# 21
```
#### Multiple numbers can also be written together
```bash
echo "4 82 2" > /dev/reducer
echo "5" > /dev/reducer
echo "8" > /dev/reducer

cat /dev/reducer
101
```
#### Using python/any other language
```python
f=open('/dev/reducer', 'w') # open in write mode
f.write('10 5')
f.close()

f=open('/dev/reducer', 'r') # open in read mode
sum_r = f.read()
print(sum_r) # o/p: 15

f.close()
```

# Installation
1. **Clone the Repository:**
```bash
git clone https://github.com/hrushikeshj/lkm-reducer.git
cd lkm-reducer
```
2. **Build the Module:**
It's recommended to have your self-compiled kernal.
```bash
make
```
3. **Load the Kernel Module**
```bash
sudo insmod reducer.ko
```
4. **Create device file**
   
Get the device number either through kernal log(`sudo dmesg`) or `/proc/devices`. The char device need not only be in `/dev`, but by convention it is placed there.
```bash
dev_no=$(cat /proc/devices | grep reducer | awk '{print $1;}')
# dev_no eg, 238
sudo mknod /dev/reducer c $dev_no 0
```
>#### Note:
>Step 3 and 4, i.e to load the module and create the device file can be easily done using the
>`install_lmk.sh` script.
>```bash
>chmod +x install_lmk.sh
>sudo ./install_lmk.sh ins reducer.ko reducer
>```
>**To Delete**
>```bash
>sudo ./install_lmk.sh del reducer.ko reducer
>```
# Reference
- [https://tldp.org/LDP/lkmpg/2.6/lkmpg.pdf](https://tldp.org/LDP/lkmpg/2.6/lkmpg.pdf)

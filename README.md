# OS161_Operating_System
Troubleshooting Thread Synchronization with OS161 Operating System


## 실행 방법
cd ~/os161/src/kern/compile/DUMBVM   
bmake depend   
bmake   
bmake install   
cd ~/os161/root   
sys161 kernel   
?   
?t  
sy1   
exit wit q   

## 출력 결과
![image](https://github.com/user-attachments/assets/88c186fc-27ae-4f9d-8c74-b5a842243df9)
![image](https://github.com/user-attachments/assets/fe7a6a60-6dba-4fc6-81c4-783c1bc962e2)
 0번 차량부터 31번 차량까지 총 32개의 차량이 직진, 우회전, 좌회전을 올바르게 Deadlock 및 Starvation 없이 진행됨을 알 수 있다.

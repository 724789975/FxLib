# ������UDP���ܲ���
------------------------

# 1.����
## 1.1 ��дĿ��
���ĵ��Ƕ�`������UDP����`����������˵����Ϊ����������е���Ӳ����Դ����϶Ը�ϵͳӦ��ģ������в��Է���,��ȱ��©����ϵͳ�ĸ�����幦��,��֤��Ŀ��˳�����У������Ա���������ʵ������Ŀ�꣺

* ��ȷ�������ܲ��ԵĲ�����Դ;
* ��ȷ�������ܲ��ԵĲ�������;
* ��ȷ�������ܲ��ԵĲ��Է���;
* ȷ�������ܲ��Ե�ϵͳ����.

## 1.2 ϵͳ����
### 1.2.1 ��Ŀ����
`��Ŀ����`: UDP���Է�����
### 1.2.2 ����Ŀ��
��ʱδ��
### 1.2.3 ����Ŀ��
��ʱδ��  

# 2.���Ի���
## 2.1 ��Ӳ������
### 2.1.1Ӳ������
* cpu:Intel(R) Xeon(R) CPU E5-2690 v3 @ 2.60GHz * 8
* menory:16G

### 2.1.2 �������
* CentOS Linux release 7.1.1503 (Core)

## 2.2 ���Ի���Լ��
* ���β��Խ������Ŀǰ����ϵͳ����/Ӳ������
* ���β��Խ������Ŀǰ����ϵͳ�ĳ���汾
* ���β��Խ������Ŀǰ����ϵͳ�����绷��
* ���β��Խ������Ŀǰ����ϵͳ�Ĳ���������

# 3.���Է�Χ������Ҫ��
## 3.1 ����
### 3.1.1 ��������
ģ�������շ����� ���Է�������500�������µ�״̬.(������Ϣ���� 768~1024 �ֽ�)
### 3.1.2��������
����һ��`TestServer`���� ������һ̨��������500��`TestClient`���� ÿ��TestClient����1������ ����ʩѹ30����(��Ҫ�����շ����ȶ����Լ�`IO`������Ч��)

### 3.1.3 ����ͨ����׼
* ϵͳ������500ʱ��ϵͳ�����ȶ�����Ϣ�������շ���ϵͳCPU��Ȼ�п���

# 4.���Խ��
## 4.1 ����������¼

* localhost.example:dnp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;=> 210.73.214.213&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;220Mb   221Mb   223Mb
* &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<=&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;211Mb   212Mb   212Mb

## 4.2 CPU״̬��¼
* VIRT&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;RES&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;SHR&nbsp;&nbsp;&nbsp;&nbsp;%CPU&nbsp;&nbsp;&nbsp;&nbsp;%MEM
* 3387800&nbsp;&nbsp;&nbsp;472908&nbsp;&nbsp;1660&nbsp;&nbsp;&nbsp;&nbsp;92.0&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.9

## 4.3 ��Ϣ����
* 500������ʱ�յ�54600������(������ϢԼ896�ֽ� ÿ��Լ47M��Ч��Ϣ)
* ������Ϣ�������յ���Ϣ��ƽ

## 4.4 ���Խ������
* ���������������������ô�� ����������`TestServer` cpu 92% ��Ԥ����Զ ������������ ������Ӧ10M/s����

# 5.����
* ���������ӽ�����ʱ ����cpu������ ����������������Ӧ�ǲ��Գ�����1/4(��ͨ�����ٿͻ��˸����������͵ĵ�����Ϣ����������) 

# 6.������Ա
* ʱ�� 2018/1/6
* ��Ա dengxiaobo
* ���� [724789975@qq.com](724789975@qq.com)


//-------------------------
// File Name: SCS_servo.c
// Author: Ma Xueyang
// Date: 2021.2.15
//-------------------------

#include "SCS_servo.h"
#include "../SCSLib/SCServo.h"
#include "../SCSLib/uart.h"
#include "../SCSLib/wiring.h"
#include "../letter_shell/src/shell_port.h"
#include "stdio.h"
#include "../mymath.h"
#include "../ee24/ee24.h"

#define POS_LEN 100
#define GROUP_LEN 30
#define GROUP_POS_LEN 15
//#define ID_START 1
//#define ID_END 5



//���ٻ�е��"λ��"������
Pos postion[POS_LEN];
//���ٻ�е��"������"������
int8_t group[GROUP_LEN][GROUP_POS_LEN]; 


/**
  * @brief  ��ʼ������
  * @retval ��
  */
void ArmInit()
{
	for(uint8_t i=0;i<GROUP_LEN;i++)
	{
		for(uint8_t j=0;j<GROUP_POS_LEN;j++)
		{
			group[i][j]=-1;
		}
	}
	for(uint8_t i=0;i<POS_LEN;i++)
  {
		postion[i].pos_id=-1;
  }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), ami, ArmInit, ArmInit());

/**
  * @brief  ����������ؿ���
  * @param  ID_:�����ƵĶ��ID
  * @param  Enable_:��0�ر������������1�����������
  * @retval ��
  */
void ArmForceEnable(uint8_t ID_,uint8_t Enable_){
	unLockEprom(ID_);//��EPROM���湦��
	EnableTorque(ID_, Enable_);
	LockEprom(ID_);//�ر�EPROM���湦��
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), amf, ArmForceEnable, ArmForceEnable(id,en));
/**
  * @brief  ȫ��������ؿ���
  * @param  Enable_:��0�ر������������1�����������
  * @retval ��
  */
void ForceAll(uint8_t Enable_){
	for(uint8_t ID_=0;ID_<5;ID_++){
		unLockEprom(ID_+1);//��EPROM���湦��
		EnableTorque(ID_+1, Enable_);
		LockEprom(ID_+1);//�ر�EPROM���湦��
	}
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), fll, ForceAll, forceAll(en));
/**
  * @brief  �洢��е�۵�ǰλ����Ϣ
  * @param  ID_:��ǰλ����Ϣ�洢�������е�λ��
  * @param  timems_:��ǰλ�õ�ִ��ʱ�䣨����ת���йأ�
  * @retval ��
  */
void SavePos(int16_t ID_,int16_t timems_)
{
	Pos postion0;
	for(uint8_t temp=0;temp<5;temp++)//��ȡ5������ĽǶ�
  {
		postion0.angle[temp]=ReadPos(temp+1);
  }
	postion0.pos_id=ID_;//���ö���id
	postion0.timems=timems_;//���ö���ʱ��
	postion[ID_]=postion0;//�洢��������
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), sap, SavePos, savePos(id,time));
/**
  * @brief  ʹ��е�۶�����ָ��λ��
  * @param  ID_:ָ��λ����Ϣ�洢�������е�λ��
  * @retval ��
  */
void GoPos(int16_t ID_)
{
	for(uint8_t temp=0;temp<5;temp++)//д5������ĽǶ�
  {
		WritePos(temp+1, postion[ID_].angle[temp], postion[ID_].timems, 0);//���(IDtemp),��ʱ��timems����,������postion[ID_].angle[temp]�Ƕ�
	}
  delay(postion[ID_].timems);//����ʽ�ȴ��������
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), gop, GoPos, goPos(id));

/**
  * @brief  �޸Ļ�������е�۶���������
  * @param  ID_:��������Ϣ�洢�������е�λ��
  * @param  PosIDs:��˳��洢�Ŷ������������������
  * @retval ��
  */
//int8_t order[]={0,1,2,3,4,5};
//saveGroup(0,order);
void Pos2Group(uint8_t G_ID_,uint8_t GP_ID_,uint8_t P_ID)
{
		group[G_ID_][GP_ID_]=P_ID;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), p2g, Pos2Group, Pos2Group(gid,gpid,pid));

/**
  * @brief  ִ�л�������ĳ��������
  * @param  ID_:��������Ϣ�洢�������е�λ��
  * @retval ��
  */
void DoGroup(uint8_t ID_)
{
	for(uint8_t i=0;group[ID_][i]!=-1;i++)
  {
		GoPos(group[ID_][i]);
  }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), dog, DoGroup, DoGroup(id));


/**
  * @brief  �洢һ��16bit���ݣ�������SaveAll2ee()������
  * @param  *cnt:Ҫ�洢�ĵ�ַ
  * @param  dt:Ҫ�洢��16bit����
  * @retval ��
  */
static void Save2ee16(uint16_t* cnt,int16_t dt)
{
	uint8_t s[2];
	s[0]=LSB(dt);
	s[1]=MSB(dt);
	ee24_write(*cnt,s,2,0xffff);
	*cnt+=2;
}
/**
  * @brief  ��ȡһ��16bit���ݣ�������readAll2ram()������
  * @param  *cnt:Ҫ��ȡ�ĵ�ַ
  * @param  dt:��ȡ�õ���16bit���ݾ��
  * @retval ��
  */
static void Readee16(uint16_t* cnt,int16_t *dt)
{
	uint8_t s[2];
	ee24_read(*cnt, s, 2, 1000);
	*dt = (int16_t) (s[1] << 8) | s[0]; 
	*cnt+=2;
}
/**
  * @brief  �����е���йصĻ�������������EEPROM��
  * @retval ��
  */
void SaveAll2ee()
{
	uint16_t lenadd=0;
	uint16_t cnt=4;//ǰ���4��λ�ã������������ֵ����ݳ���
	if (ee24_isConnected()){
		
		//----��ʼ��λ�ã����1400�ֽ�
		for(uint8_t i=0;postion[i].pos_id!=-1;i++)//����λ��ID
		{
			Save2ee16(&cnt,postion[i].pos_id);//��ID
			for(uint8_t j=0;j<5;j++)
			{
				Save2ee16(&cnt,postion[i].angle[j]);//������
			}
			Save2ee16(&cnt,postion[i].timems);//�涯��ʱ��
		}
		//��λ�ý���
		Save2ee16(&lenadd,cnt);//��EEPROM�Ŀ�ͷ��λ�������ܳ���
		
		//----��ʼ�涯���飬���450�ֽ�
		for(uint8_t i=0;group[i][0]!=-1;i++)//����������ID
		{
			ee24_write(cnt,(uint8_t*)group[i],GROUP_POS_LEN,0xffff);//�洢�ö������λ������
			cnt+=GROUP_POS_LEN;
		}
		//�涯�������
		Save2ee16(&lenadd,cnt);//��EEPROM�Ŀ�ͷ�涯���������ܳ���
	}
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), sa, SaveAll2ee, SaveAll2ee());

/**
  * @brief  ��EEPROM�е����ݶ���������
  * @retval ��
  */
void readAll2ram()
{
	uint16_t cnt=0;
	int16_t len[2];
	if (ee24_isConnected()){
		//�����ݳ���
		Readee16(&cnt,len);//λ�������ܳ���
		Readee16(&cnt,len+1);//�����������ܳ���
		
		//----��ʼ��λ��----
		for(uint8_t i=0;i<len[0]/sizeof(postion[0]);i++)//����λ��ID
		{
			Readee16(&cnt,&postion[i].pos_id);//��ID
			for(uint8_t j=0;j<5;j++)
			{
				Readee16(&cnt,&postion[i].angle[j]);//�������
			}
			Readee16(&cnt,&postion[i].timems);//��ID
		}
		//----��ʼ��������----
		for(uint8_t i=0;i<len[1]/GROUP_POS_LEN;i++)//����������ID
		{
			ee24_read(cnt,(uint8_t*)group[i],GROUP_POS_LEN,0xffff);//��ȡ�ö������λ������
			cnt+=GROUP_POS_LEN;
		}
	}
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), ra, readAll2ram, readAll2ram());

/**
  * @brief  5�����ȫ������
  * @retval ��
  */
void ArmGoMiddle()
{
	for(uint8_t temp=0;temp<5;temp++)
  {
		WritePos(temp+1, 510, 1000, 0);
  }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), mi, ArmGoMiddle, ArmGoMiddle());

/*--------letter shell example begin--------*/
//void fun(char en)
//{
//    printf("Test function��\r\n");
//		ArmForceEnable(2,en);
//}
////����func ��ִ��fun
//SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), func, fun, test);
/*--------letter shell example end--------*/

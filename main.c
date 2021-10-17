#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#define total_instruction 320 //ָ��������ָ��������
#define total_vp 32 //��ҳ��������ռ��С��
#define INVALID -1
#define TRUE 1
#define FALSE 0

#define clear_period 50 //�������ڣ�NUR��

typedef struct
{
    int pn; //page number
    int pfn; //page_frame number
    int counter; //���ʼ�¼��LFU��NUR��
    int time; //����ʱ�䣨LRU��
}pl_type;//ҳ��ṹ�壨���̣�
pl_type pl[32];//32�����ҳ��

typedef struct pfc_struct
{
    int pn, pfn;
    struct pfc_struct *next;
}pfc_type;//ҳ��ṹ�壨�ڴ棩
pfc_type pfc[32], *freepf_head, *busypf_head, *busypf_tail;

int diseffect; //ȱҳ����
int a[total_instruction]; //��ַ����
int page[total_instruction]; //ҳ��
int offset[total_instruction]; //ҳ��ƫ��

void generate_inst_addr_sq(){ //���ɵ�ַ����
    int s, i;
    srand(10*getpid()); //�������������

    s=(float)319*rand()/RAND_MAX+1; //0-319�����
    for(i=0;i<total_instruction;i+=4)
    {
        if(s<0||s>319)
        {
            printf("When i==%d, Error, s==%d\n", i,s);
            exit(0);
        }
        a[i]=s; //����һָ����ʵ�
        a[i+1]=a[i]+1; //˳��ִ��һ��ָ��
        a[i+2]=(float)a[i]*rand()/RAND_MAX; //ִ��ǰָ��m
        a[i+3]=a[i+2]+1; //˳��ִ��һ��ָ��
        s=(float)(318-a[i+2])*rand()/RAND_MAX+a[i+2]+2;
        if((a[i+2]>318)||(s>319))
            printf("a[%d+2], a number which is: %d and s==%d", i, a[i+2], s);
    }
    for(i=0;i<total_instruction;i+=4)
        printf("%03d, %03d, %03d, %03d\n", a[i], a[i+1], a[i+2], a[i+3]);

    for(i = 0; i<total_instruction; i++)
    {//����ַת��Ϊҳ�š�ҳ��ƫ�ƣ�ÿ��ҳ���СΪ10
        page[i] = a[i]/10;
        offset[i] = a[i]%10;
    }
}

void initialize(int total_pf)
{
    int i;
    diseffect = 0;

    for(i = 0; i<32; i++)
    {
        pl[i].pn = i;
        pl[i].pfn = INVALID;
        pl[i].counter = 0;
        pl[i].time = -1;
    }
    for(i = 0; i<total_pf-1; i++)
    {
        pfc[i].next = &pfc[i+1];
        pfc[i].pfn = i;
    }
    pfc[total_pf-1].next = NULL;
    pfc[total_pf-1].pfn = total_pf - 1;
    freepf_head = &pfc[0]; //freepf_head����ָ�����ҳ��
}

void FIFO(int total_pf)
{
    int i, j;
    pfc_type *p;
    initialize(total_pf);
    busypf_head=busypf_tail=NULL;
    for(i=0;i<total_instruction;i++)
    {
        if(pl[page[i]].pfn==INVALID)
        {
            diseffect+=1;
            if(freepf_head==NULL) //û�п���ҳ��
            {//�ͷ�æҳ������еĵ�һ��ҳ���
                p=busypf_head->next;
                pl[busypf_head->pn].pfn=INVALID; //ԭ��ҳȡ������ҳ��
                freepf_head=busypf_head; 
                freepf_head->next=NULL; //����p=freepf_head->next��֮�󱣳�û�п���ҳ��
                busypf_head=p; //����æҳ��������
            }
            //freepf��ʾҳ������ҳ��
            p=freepf_head->next; //�ݴ���һ�����ܿ��е�ҳ��
            freepf_head->next=NULL;
            freepf_head->pn=page[i];
            pl[page[i]].pfn=freepf_head->pfn;
            /*��freepfҳ�����æ����β*/
            if(busypf_tail==NULL) //æҳ�����Ϊ�գ���ִ��һ�Σ�
                busypf_head=busypf_tail=freepf_head;
            else
            {
                busypf_tail->next=freepf_head;
                busypf_tail=freepf_head;
            }
            freepf_head=p;
        }
    }
    printf("FIFO:%6.4f ",1-(float)diseffect/320);
}

void LRU(int total_pf)
{
    int min, minj, i, j, present_time;
    initialize(total_pf);
    present_time=0;

    for(i=0; i<total_instruction;i++)
    {
        if(pl[page[i]].pfn==INVALID)
        {
            diseffect++;
            if(freepf_head==NULL) //û�п���ҳ�����滻
            {   //�ҵ�time��С��ҳ�����ͷ���ҳ��
                min=32767;
                for(j=0;j<32;j++)
                {
                    if(min>pl[j].time&&pl[j].pfn!=INVALID) //ҳ�������time��С��ҳ��
                    {
                        min=pl[j].time;
                        minj=j;
                    }
                }
                //��ҳ�����free_pf������time��С��ҳ��
                freepf_head=&pfc[pl[minj].pfn];
                pl[minj].pfn=INVALID;
                pl[minj].time=-1;
                freepf_head->next=NULL; //����freepf_head=freepf_head->next��֮�󱣳�û�п���ҳ��
            }
            //��ҳ����freepf�еĵ�һ��ҳ��
            pl[page[i]].pfn=freepf_head->pfn;
            pl[page[i]].time=present_time;
            freepf_head=freepf_head->next;
        }
        else
            pl[page[i]].time=present_time; //æҳ����������У�����time
        present_time++;
    }
    printf("LRU:%6.4f ",1-(float)diseffect/320);
}

/*void NUR(int total_pf)
{
    int i, j, dp, cout_flag, old_dp;
    pfc_type *t;
    
    initialize(total_pf);
    dp=0;
    for(i=0;i<total_instruction;i++)
    {
        if(pl[page[i]].pfn==INVALID)
        {
            diseffect++;
            if(freepf_head==NULL)
            {
                cout_flag=TRUE;
                old_dp=dp;
                while(cout_flag)
                    if(pl[dp].counter==0&&pl[dp].pfn!=INVALID)
                        cout_flag=FALSE;
                    else
                    {
                        dp++;
                        if(dp==total_vp)
                            dp=0;
                        if(dp==old_dp)
                        {
                            for(j=0;j<total_vp;j++)
                                pl[j].counter=0;
                        }
                    }
                freepf_head=&pfc[pl[dp].pfn];
                pl[dp].pfn=INVALID;
                freepf_head->next=NULL;
            }
            pl[page[i]].pfn=freepf_head->pfn;
            freepf_head=freepf_head->next;
        }
        else
            pl[page[j]].counter=1;
        if(i%clear_period==0)
            for(j=0;j<total_vp;j++)
                pl[j].counter=0;
    }
    printf("NUR:%6.4f ",1-(float)diseffect/320);
}*/

/*void OPT(int total_pf)
{
    int i,j , max, maxpage, d, dist[total_vp];
    pfc_type *t;
    initialize(total_pf);
    for(i=0;i<total_instruction;i++)
    {
        if(pl[page[i]].pfn==INVALID)
        {
            diseffect++;
            if(freepf_head==NULL)
            {
                for(j=0;j<total_vp;j++)
                    if(pl[j].pfn!=INVALID)
                        dist[j]=32767;
                    else
                        dist[j]=0;
                d=1;
                for(j=i+1;j<total_instruction;j++)
                {
                    if(pl[page[j]].pfn!=INVALID)
                        dist[page[j]]=d;
                    d++;
                }
                max=-1;
                for(j=0;j<total_vp;j++)
                    if(max<dist[j])
                    {
                        max=dist[j];
                        maxpage=j;
                    }
                freepf_head=&pfc[pl[maxpage].pfn];
                freepf_head->next=NULL;
                pl[maxpage].pfn=INVALID;
            }
            pl[page[i]].pfn=freepf_head->pfn;
            freepf_head=freepf_head->next;
        }
    }
    printf("OPT:%6.4f ",1-(float)diseffect/320);
}*/

/*void LFU(int total_pf)
{
    int i,j,min,minpage;
    pfc_type *t;

    initialize(total_pf);
    for(i=0;i<total_instruction;i++)
    {
        if(pl[page[i]].pfn==INVALID)
        {
            diseffect++;
            if(freepf_head==NULL)
            {
                min=32767;
                for(j=0;j<total_vp;j++)
                {
                    if(min>pl[j].counter&&pl[j].pfn!=INVALID)
                    {
                        min=pl[j].counter;
                        minpage=j;
                    }
                    pl[j].counter=0;
                }
                freepf_head=&pfc[pl[minpage].pfn];
                pl[minpage].pfn=INVALID;
                freepf_head->next=NULL;
            }
            pl[page[i]].pfn=freepf_head->pfn;
            freepf_head=freepf_head->next;
            pl[page[i]].counter++;
        }
        else
            pl[page[i]].counter++;
    }
    printf("LFU:%6.4f ",1-(float)diseffect/320);
}*/

int main()
{
    int i;

    generate_inst_addr_sq();

    for(i=4;i<=32;i++)//�����ڴ�������4-32��ҳ��
    {
        printf("%2d page frames\t",i);
        FIFO(i);
        LRU(i);
        //OPT(i);
        //LFU(i);
        //NUR(i);
        printf("\n");
    }
    return 0;
}

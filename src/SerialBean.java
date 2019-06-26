//import java.io.IOException;
import javax.comm.CommPortIdentifier;
import javax.comm.SerialPort;
import javax.comm.UnsupportedCommOperationException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Com21PollingListener类使用“轮训”的方法监听串口COM21，
 * 并通过COM21的输入流对象来获取该端口接收到的数据（在本文中数据来自串口COM11）。
 */

public class SerialBean {

    public static void main(String[] args){
        int blinkcount=0;
        int checksum = 0;
        int generatedChecksum = 0;
        int xxhigh=0;
        int xxlow=0;
        int rawdata=0;
        int attention = 0;
        int meditation = 0;
        boolean bigPacket = false;
        boolean blinking = false;
        int blinkFlag=0;
        //1.定义变量
        CommPortIdentifier com_input = null;
        CommPortIdentifier com_output = null;
        SerialPort serialCom5 = null;//打开的端口
        SerialPort serialCom38 = null;
        InputStream inputStream = null;//端口输入流
        OutputStream outputStream=null;
        try{
            //2.获取并打开串口COM21
            com_input = CommPortIdentifier.getPortIdentifier("COM5");
            com_output = CommPortIdentifier.getPortIdentifier("COM38");
            serialCom5 = (SerialPort) com_input.open("Com5Listener", 1000);
            serialCom38 = (SerialPort) com_output.open("Com38Listener", 1000);
            try {
                serialCom5.setSerialPortParams(
                        57600, //波特率
                        SerialPort.DATABITS_8,//数据位数
                        SerialPort.STOPBITS_1,//停止位
                        SerialPort.PARITY_NONE//奇偶位
                );
            }catch (UnsupportedCommOperationException e) {
                e.printStackTrace();
            }

            inputStream = serialCom5.getInputStream();
            int i,j;

            while(true) {
                if (inputStream.read() == 170) {  //AA
                    if (inputStream.read() == 170) {    //AA
                        switch (inputStream.read()) {
                            case 4://04              处理小包数据
                                if (inputStream.read() == 128) {    //80
                                    if (inputStream.read() == 2) {    //02
                                        generatedChecksum = 130;
                                        xxhigh = inputStream.read();
                                        xxlow = inputStream.read();
                                        generatedChecksum += xxhigh + xxlow;
                                        checksum = inputStream.read();
                                        generatedChecksum = 255 - generatedChecksum;   //计算生成的校验和
                                        if (checksum == generatedChecksum) {  //如果算出来的sum和xxCheckSum是相等的，那说明这个包是正确
                                            rawdata = xxhigh * 256 + xxlow;
                                            if (rawdata >= 32768) {
                                                rawdata = rawdata - 65536;
                                            }
                                            if (rawdata > 1400) {   //用是否眨眼控制灯的亮灭

                                                blinkcount++;
                                                blinking = true;
                                            }
                                        } else {
                                        }
                                    }
                                }
                                break;

                            case 32: //20     处理大包数据
                                //大包做校验和
                                //generatedChecksum=0;
                                //30次循环是因为原本有32次循环，判断attention
                                //和meditation一次读取了两个字节，少了两次循环
                                for ( i = 0; i < 32; i++) {
                                    if (inputStream.read() == 4) {
                                        attention = inputStream.read();
                                    }
                                    if (inputStream.read() == 5) {
                                        meditation = inputStream.read();
                                    }
                                }
                                if (attention >= 0 && attention <= 100 && meditation >= 0 && meditation <= 100) {
                                    bigPacket = true;
                                }
                                break;

                            default:
                                break;
                        }
                        if(bigPacket) {
                            if(attention!=0&&meditation!=0){
                                System.out.print("Rawdata :");
                                if(blinking){
                                    System.out.print("blinked with ");
                                    System.out.print(blinkcount);
                                    if(blinkcount<65){
                                        //count值小于65被判定为有效数据
                                        blinkFlag=1;
                                    }
                                }else{
                                    System.out.print("no blinking ");
                                }
                                System.out.print(" Attention: ");
                                System.out.print(attention);
                                System.out.print(" Meditation: ");
                                System.out.print(meditation);
                                System.out.print("\n");

                                outputStream=serialCom38.getOutputStream();
                                outputStream.write(blinkFlag);
                            }else{
                                System.out.print("无效数据帧");
                                System.out.print("\n");
                            }
                            blinking=false;
                            blinkcount=0;
                            blinkFlag=0;
                        }

                        bigPacket = false;
                    }
                }
            }
        }catch (Exception e) {
            //如果获取输出流失败，则抛出该异常
            e.printStackTrace();
        }


    }
}

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ScreamD
{
    public partial class Form1 : Form
    {
        public float screamProbability = 0.0f;
        public string locationData = "";
        public string bearingData = "";
        public int lowerFreq = 200;
        public int upperFreq = 1000;
        public byte[] gainCoeffs = new byte[8];
        public float thresholdFFT = 300.0f;
        public int meanPower = 10;
        public byte[] fftData = new byte[256];
        public byte dataComing = 0;
        public delegate void addTextDelegate(string data);
        public addTextDelegate myDelegate;
        public byte[] samples = new byte[2048];
        public UInt16 sample_c = 0;
        public byte checksum = 0;
        public byte working = 0;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            this.myDelegate = new addTextDelegate(addText);
        }

        private void button10_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                serialPort1.Close();
                button10.Text = "Открыть COM";
            }
            else
            {
                serialPort1.PortName = textBox2.Text;
                try
                {
                    serialPort1.Open();
                    if (serialPort1.IsOpen)
                    {
                        button10.Text = "Закрыть COM";
                    }
                }
                catch (Exception)
                {

                }
            }
        }

        private void button9_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch(Exception)
            {

            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        private void button7_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        private void button8_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                if (serialPort1.IsOpen)
                {

                }
            }
            catch (Exception)
            {

            }
        }

        public void addText(string data)
        {
            textBox17.AppendText(data);
        }

        private void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            while (serialPort1.BytesToRead>0)
            {
                byte res = (byte)serialPort1.ReadByte();
                
                if (dataComing == 0)
                {
                    if (res == 0xAA)
                    {
                        textBox17.Invoke(this.myDelegate, "DATA\n");
                        dataComing = 1;
                        sample_c = 0;
                        checksum = 0xAA;
                        working = 1;
                    }
                    if (working == 0)
                    {
                        string r = "";
                        r += (char)res;
                        textBox17.Invoke(this.myDelegate, r);
                    }
                }
                else
                {
                    samples[sample_c] = res;
                    checksum ^= res;
                    sample_c++;
                    if (sample_c == 101)
                    {
                        if (checksum==0)
                        {
                            textBox17.Invoke(this.myDelegate, "READ OK\n");
                        }
                        dataComing = 0;
                    }
                }
            }
        }

        private void button11_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                byte[] start = new byte[1];
                start[0] = 0xAA;
                byte data;
                byte.TryParse(textBox4.Text, out data);
                gainCoeffs[0] = data;
                byte.TryParse(textBox5.Text, out data);
                gainCoeffs[1] = data;
                byte.TryParse(textBox6.Text, out data);
                gainCoeffs[2] = data;
                byte.TryParse(textBox9.Text, out data);
                gainCoeffs[3] = data;
                byte.TryParse(textBox8.Text, out data);
                gainCoeffs[4] = data;
                byte.TryParse(textBox7.Text, out data);
                gainCoeffs[5] = data;
                byte.TryParse(textBox11.Text, out data);
                gainCoeffs[6] = data;
                byte.TryParse(textBox10.Text, out data);
                gainCoeffs[7] = data;
                byte[] data2 = new byte[7];
                UInt16 data_i;
                UInt16.TryParse(textBox13.Text, out data_i);
                data2[0] = (byte)((data_i & 0xFF00) >> 8);
                data2[1] = (byte)(data_i & 0xFF);
                UInt16.TryParse(textBox14.Text, out data_i);
                data2[2] = (byte)((data_i & 0xFF00) >> 8);
                data2[3] = (byte)(data_i & 0xFF);
                UInt16.TryParse(textBox15.Text, out data_i);
                data2[4] = (byte)((data_i & 0xFF00) >> 8);
                data2[5] = (byte)(data_i & 0xFF);
                byte.TryParse(textBox16.Text, out data);
                data2[6] = data;
                serialPort1.Write(start,0,1);
                serialPort1.Write(gainCoeffs, 0, 8);
                serialPort1.Write(data2, 0, 7); 
            }
        }
    }
}

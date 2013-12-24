package org.keves.ledstrip;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import afzkl.development.colorpickerview.view.ColorPickerView;
import afzkl.development.colorpickerview.view.ColorPickerView.OnColorChangedListener;
import android.net.DhcpInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity implements OnColorChangedListener {

	DatagramSocket sock;
    InetAddress addr;
    EditText ipAddrText;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        try {
			sock = new DatagramSocket();
			//addr = InetAddress.getByName("192.168.0.103");
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
        ColorPickerView mColorPickerView = (ColorPickerView) findViewById(R.id.colorPickerView1);
        mColorPickerView.setOnColorChangedListener(this);
        
        ipAddrText = (EditText) findViewById(R.id.textIpAddr);
        
        Button btnFind = (Button) findViewById(R.id.btnFind);
        btnFind.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				new Thread(new Runnable() {
		            public void run() {
		                threadSearchBeacon();
		            }
		        }).start();		       				
			}
		});
        
        Log.e("XXX", "Haaaai!");
        
        new Thread(new Runnable() {
            public void run() {
            	try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
                threadSearchBeacon();
            }
        }).start();	
        
        new Thread(new Runnable() {
            public void run() {
                threadMain();
            }
        }).start();
        
    }
    
    InetAddress getBroadcastAddress() throws IOException {
        WifiManager wifi = (WifiManager) this.getSystemService(Context.WIFI_SERVICE);
        DhcpInfo dhcp = wifi.getDhcpInfo();
        // handle null somehow

        int broadcast = (dhcp.ipAddress & dhcp.netmask) | ~dhcp.netmask;
        byte[] quads = new byte[4];
        for (int k = 0; k < 4; k++)
          quads[k] = (byte) ((broadcast >> k * 8) & 0xFF);
        
        //quads[0] = (byte)192; quads[1] = (byte)168; quads[2] = (byte)0; quads[3] = (byte)103;
        
        return InetAddress.getByAddress(quads);
    }
    
    void threadSearchBeacon() {
    	InetAddress bcastAddr;
		try {
			// get address
			bcastAddr = getBroadcastAddress();
	    	Log.e("XXX", "Bcast: "+bcastAddr);
	    	
	    	// send request
	    	DatagramSocket socket = new DatagramSocket();
	    	socket.setBroadcast(true);
	    	socket.setSoTimeout(1000);
	    	String data = "HELL0";
	    	DatagramPacket packet = new DatagramPacket(data.getBytes(), data.length(),
	    	    getBroadcastAddress(), 9023);
	    	socket.send(packet);
	    	
	    	
	    	// get response
	    	byte[] buf = new byte[1024];
	        try {
	          while (true) {
	            final DatagramPacket packet2 = new DatagramPacket(buf, buf.length);
	            socket.receive(packet2);
	            String s = new String(packet2.getData(), 0, packet2.getLength());
	            Log.d("XXX", "Received response " + s);
	            if (s.equals("OK!")) {
	            	Log.d("XXX", "Got RASP on "+packet2.getAddress());
	            	
	            	ipAddrText.post(new Runnable() {
	            	    public void run() {
	            	        ipAddrText.setText(packet2.getAddress().toString().substring(1));
	            	    } 
	            	});
	            	
	            	break;
	            }
	          }
	        } catch (SocketTimeoutException e) {
	          Log.d("XXX", "Receive timed out");
	        }
	    	
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return;
		}

    }
    
    final Lock lock = new ReentrantLock();
    final Condition needSend  = lock.newCondition();
    int last_r, last_g, last_b;
    void threadMain() {
    	for (;;) {
    		lock.lock();
    		try
    		{
	    		try {
					needSend.await();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					return;
				}
	    		
	    		send_color(last_r, last_g, last_b);
    		}
    		finally
    		{
    			lock.unlock();
    		}
    	}
    }
    
    void send_color(int r, int g, int b) {
    	int n_leds = 229;
    	byte[] buf = new byte[3 * n_leds];
    	
    	for (int i = 0; i < n_leds; ++i) {
    		buf[(i * 3) + 0] = (byte) r;
    		buf[(i * 3) + 1] = (byte) g;
    		buf[(i * 3) + 2] = (byte) b;
    	}
    	
    	try {
			addr = InetAddress.getByName(ipAddrText.getText().toString());
		} catch (UnknownHostException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
			return;
		}
    	
    	DatagramPacket sendPacket = new DatagramPacket(buf, buf.length, addr, 9022);
        try {
			sock.send(sendPacket);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }


	@Override
	public void onColorChanged(int newColor) {
		int r = Color.red(newColor);
		int g = Color.green(newColor);
		int b = Color.blue(newColor);
		Log.e("XXX", "R:"+r+" G:"+g+" B:"+b);
	//	send_color(r,g,b);
		last_r = r;
		last_g = g;
		last_b = b;
		lock.lock();
		needSend.signal();
		lock.unlock();
	}

}

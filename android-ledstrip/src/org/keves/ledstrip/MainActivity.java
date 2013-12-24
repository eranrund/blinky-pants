package org.keves.ledstrip;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import afzkl.development.colorpickerview.view.ColorPickerView;
import afzkl.development.colorpickerview.view.ColorPickerView.OnColorChangedListener;
import android.os.Bundle;
import android.app.Activity;
import android.graphics.Color;
import android.util.Log;
import android.view.Menu;

public class MainActivity extends Activity implements OnColorChangedListener {

	DatagramSocket sock;
    InetAddress addr;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        try {
			sock = new DatagramSocket();
			addr = InetAddress.getByName("192.168.0.103");
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
        ColorPickerView mColorPickerView = (ColorPickerView) findViewById(R.id.colorPickerView1);
        mColorPickerView.setOnColorChangedListener(this);
        Log.e("XXX", "Haaaai!");
        
        new Thread(new Runnable() {
            public void run() {
                threadMain();
            }
        }).start();
        
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

/*
 * EnigmaLight (c) 2014 Speedy1985, Oktay Oeztueter (Based on Boblight from Bob Loosen)
 * parts of this code were taken from
 *
 * - aiograb		(http://schwerkraft.elitedvb.net/projects/aio-grab/)
 * - enigmalight (c) 2009 Bob Loosen
 * 
 * EnigmaLight is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * EnigmaLight is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 //

// General includes
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>

// Util includes
#include "Util/Inclstdint.h"
#include "Util/Misc.h"
#include "Util/TimeUtils.h"
#include "Util/Lock.h"
#include "Util/Log.h"

// Grabber includes
#include "Grabber/FrameGrabber.h"

using namespace std;

CStb m_stb;

CFrameGrabber::CFrameGrabber(CGrabber* grabber) : m_grabber(grabber)
{	
	xres_tmp    = 0;
	yres_tmp    = 0;
	m_last_res_process  = 0.0;
}

bool CFrameGrabber::Setup()
{
	try
  	{
		//Detect stb
		if (!m_stb.DetectSTB()) { //If unknown then return.
			LogError("STB Detection failed!");
			return false; //Stop enigmalight
		}
		else
		{      
	        if(m_grabber->m_debug){
	            Log("DBG -> settings: chr_luma_stride %x", m_stb.chr_luma_stride);
	            Log("DBG -> settings: chr_luma_register_offset %x", m_stb.chr_luma_register_offset);
	            Log("DBG -> settings: registeroffset %x", m_stb.registeroffset);
	            Log("DBG -> settings: mem2memdma_register %x", m_stb.mem2memdma_register);
	        }
	        
		if(m_grabber->m_3d_mode != 1)
	    	Log("3D mode: %i",m_grabber->m_3d_mode);
		if (m_grabber->m_debug) 
			Log("Debug mode: enabled");

		// Set some vars to default values
		m_errorGiven = false;
		m_fps=0;

		if(m_grabber->m_debug)
		{
		    //Set fps counters
		    fps_lastupdate		= GetTimeUs();
		    fps_framecount		= 0;
		    m_lastupdate		= GetTimeSec<long double>();
		    m_lastmeasurement	= m_lastupdate;
		    m_measurements 		= 0.0;
		    m_nrmeasurements 	= 0.0;
		}

		return true; // All ok? then return true and start Run();
  	}
	catch (string &error)
  	{
    	PrintError(error);
    	return false;
  	}
}

#define PERROR(R, C, S) (R) = (C); if ((R) < 0) { perror((S)); exit(errno); }

ssize_t readall(int fd, void *buf, ssize_t count) {
	ssize_t nbytes, bytesleft = count;
	char *p = (char *)buf;
	while (bytesleft) {
		PERROR(nbytes, read(fd, p, bytesleft), "read");
		if (nbytes == 0) break; // EOF
		bytesleft -= nbytes;
		p += nbytes;
	}
	return count - bytesleft;
}

bool CFrameGrabber::grabFrame(CBitmap* bitmap, int skiplines)
{
	m_noVideo = true;
	int xres_orig = 1920;
	int yres_orig = 1080;
	int skipres = yres_orig;
	if(xres_orig > yres_orig)
		skipres = xres_orig;
	while(skipres/skiplines > 128){
		skiplines *= 2;
	}
	if (yres_orig%2 == 1)
		yres_orig--;
	bitmap->SetYres(yres_orig/skiplines);
	bitmap->SetXres(xres_orig/skiplines);
	bitmap->SetYresOrig(yres_orig);
	bitmap->SetXresOrig(xres_orig);
	int r, outpipe[2], errpipe[2];
	pid_t pid;
	PERROR(r, pipe(outpipe), "pipe");
	PERROR(r, pipe(errpipe), "pipe");
	PERROR(pid, fork(), "fork");
	if (pid == 0) {
		//child
		dup2 (outpipe[1], STDOUT_FILENO);
		close(outpipe[0]);
		close(outpipe[1]);
		dup2 (errpipe[1], STDERR_FILENO);
		close(errpipe[0]);
		close(errpipe[1]);
		PERROR(r, execl("/usr/bin/grab", "grab", "-v", "-s", (char *)0), "execl");
	} else {
		// parent
		unsigned char buf[1920 * 1080 * 3 + 54];
		readall(outpipe[0], buf, 1920 * 1080 * 3 + 54);
		close(outpipe[1]);
		close(errpipe[1]);
		close(errpipe[0]);
		close(outpipe[0]);
		wait(NULL);
		unsigned char *bmp = buf + 54; // skip bmp header
		if (bitmap->m_data) {
			free(bitmap->m_data);
		}
		int xres = bitmap->GetXres(), yres = bitmap->GetYres();
		if (!(bitmap->m_data = (unsigned char *)malloc(xres * yres * 3))) {
			Log("out of memory");
			exit(EXIT_FAILURE);
		}
		for (int y = 0; y < yres; ++y) {
			for (int x = 0; x < xres; ++x) {
				int src_offset = 3 * (1920 * ((yres - 1 - y) * skiplines + skiplines / 2) + (x * skiplines + skiplines / 2));
				int dst_offset = 3 * (xres * y + x);
				bitmap->m_data[dst_offset]     = bmp[src_offset + 2];
				bitmap->m_data[dst_offset + 1] = bmp[src_offset + 1];
				bitmap->m_data[dst_offset + 2] = bmp[src_offset];
			}
		}
		m_noVideo = false;
		/*
		int im = open("/tmp/s.dat", O_WRONLY | O_CREAT);
		if (im < 0) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		write(im, bitmap->m_data, xres * yres * 3);
		close(im);
		*/
	}
	return true;
}

void CFrameGrabber::getResolution(CBitmap* bitmap, int stride, long double now)
{
    ///Check for resolution every 10 seconds or if xres/yres is not ok    
    if (now - m_last_res_process >= 10.0 || m_last_res_process <= 0.0 || stride != xres_tmp || yres_tmp <= 0 || xres_tmp <= 0){
        m_last_res_process = now;
        
	    // get resolutions from the proc filesystem and save it to tmpvar
	    yres_tmp = 1080; //hexFromFile("/proc/stb/vmpeg/0/yres");
	    xres_tmp = 1920; //hexFromFile("/proc/stb/vmpeg/0/xres");
    }
    
    // Save orginal resolution
    bitmap->SetXresOrig(xres_tmp);
    bitmap->SetYresOrig(yres_tmp);
}

void CFrameGrabber::sendBlank(CBitmap* bitmap)
{
	int xres = bitmap->GetXres();
	int yres = bitmap->GetYres();

    // Set black
    m_stateBlank = true;

    // Set new bitmap size
    bitmap->SetData((unsigned char*)malloc(xres*yres*4), xres, yres);
    
    m_grabber->m_enigmalight->ProcessImage(&bitmap->m_data[0], xres, yres, m_grabber->m_delay);  //send bitmap, there it will filter al the values  
    if (!m_grabber->SendRGB(1, NULL,m_grabber->m_cluster))
        PrintError(m_grabber->m_enigmalight->GetError());
    
    yres_tmp=xres_tmp=0;
    
    if(m_grabber->m_debug)
        Log("Nothing to grab, Lights off");
        
}

bool CFrameGrabber::CheckRes(CBitmap* bitmap)
{	
	// Scaled resolution
	int xres = bitmap->GetXres();
	int yres = bitmap->GetYres();

	// Old saved resolution
	int yres_old = bitmap->GetYresOld();
	int xres_old = bitmap->GetXresOld();
	
	// Orginal resolution
	int xres_orig = bitmap->GetXresOrig();
	int yres_orig = bitmap->GetYresOrig();

	if  (m_old_3d_mode != m_grabber->m_3d_mode || (yres_old != yres) || (xres_old != xres) 
    	|| yres <= 1 || yres >= yres_orig/2 || xres <= 1 || xres >= xres_orig/2) 
    {  
        m_noVideo = true;
        if(m_old_3d_mode != m_grabber->m_3d_mode || (xres > 2 && yres > 2))
        {   
            bitmap->SetYresOld(yres); bitmap->SetXresOld(xres);

            if(xres != xres_orig && yres != yres_orig && yres_orig > 0 && xres_orig > 0)
    		{
    			//
    			// Set new scanrange
    			//
			    if(m_grabber->m_3d_mode == 1)
			    {
			        m_grabber->m_enigmalight->SetScanRange(xres, yres); //normal
			        Log("Set Scanrange to %dx%d (Source %dx%d)",xres,yres,xres_orig,yres_orig);
			    }
			    else if(m_grabber->m_3d_mode == 2)
			    {
			        m_grabber->m_enigmalight->SetScanRange(xres, yres/2); //topandbottom
			        Log("Set Scanrange to %dx%d (Source %dx%d)",xres,yres/2,xres_orig,yres_orig/2);
			        Log("3D Mode: TAB");        
			    }
			    else if(m_grabber->m_3d_mode == 3)
			    {
			        m_grabber->m_enigmalight->SetScanRange(xres/2, yres); //sidebyside
			        Log("Set Scanrange to %dx%d (Source %dx%d)",xres/2,yres,xres_orig/2,yres_orig);
			        Log("3D Mode: SBS");
			    }    
    

    			//Saves the 3dmode to check every loop for changes
    			m_old_3d_mode = m_grabber->m_3d_mode;
    			
	            // Remove old bitmap and malloc a newone
	        	bitmap->SetData((unsigned char*)malloc(bitmap->GetXres()*bitmap->GetYres()*4), xres, yres);
	     	}       
        }
    }

    //
    // If there is no video or xres,yres are 1 or lower then send blankFrame
    //
    if(m_noVideo || yres <= 1 || xres <= 1){
        
        //If set_black is false then send once black to lights
        if(!m_stateBlank)
        {
        	sendBlank(bitmap);
        }
        
        #define SLEEPTIME 100000
		
		USleep(SLEEPTIME, &m_noVideo);

        return true; // Video is blank
    }
    else{
      
        // Set state m_noVideo to false
        m_stateBlank = false;

        // Set to false so we can receive new errors.
        m_errorGiven = false; 		    	
    }

    return false; // Video is not blank
}

void CFrameGrabber::updateInfo(CBitmap* bitmap, CGuiServer& g_guiserver)
{
	long double now = GetTimeSec<long double>(); 		// current timestamp
	m_measurements += now - m_lastmeasurement;			// diff between last time we were here
	m_nrmeasurements++;									// got another measurement
	m_lastmeasurement = now;							// save the timestamp
	
	if (now - m_lastupdate >= 1.0)						// if we've measured for one second, place fps on ouput.
	{
		m_lastupdate = now;

		if (m_nrmeasurements > 0) 
			m_fps = 1.0 / (m_measurements / m_nrmeasurements); // we need at least one measurement
		
		m_measurements = 0.0;
		m_nrmeasurements = 0;

		g_guiserver.SetInfo(m_fps,bitmap->GetXres(),bitmap->GetYres(),bitmap->GetXresOrig(),bitmap->GetYresOrig());
		
		if(m_grabber->m_debug)
		{
		    if(!m_noVideo){
		         Log("DBG -> gFPS:%2.1Lf | Res:%dx%d (%dx%d)",m_fps,bitmap->GetXres(),bitmap->GetYres(),bitmap->GetXresOrig(),bitmap->GetYresOrig());
		    }else{
		         Log("DBG -> gFPS:%2.1Lf | No video input...",m_fps);
		    }
		}
	}
}

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.lang.String.*;
/**
  This class demonstrates the basics of setting up a Java Swing GUI uisng the
  BorderLayout. You should be able to use this program to drop in other
  components when building a GUI
  */



public class TCPClientGUI{
  // Initialize all swing objects.
  private JFrame f = new JFrame("OV Command GUI"); //create Frame
  private JPanel pnlNorth = new JPanel(); // North quadrant
  private JPanel pnlSouth = new JPanel(); // South quadrant
  private JPanel pnlEast = new JPanel(); // East quadrant
  private JPanel pnlWest = new JPanel(); // West quadrant
  private JPanel pnlCenter = new JPanel(); // Center quadrant

  // Buttons some there is something to put in the panels
  private JButton btnNorth = new JButton("startOVRC/restartOVRC");
  private JButton btnSouth = new JButton("start OV Calibration");
  private JButton btnEast = new JButton("Kill ALL ONLINE");
  private JButton btnWest = new JButton("Kill OV Calibration");
  private JButton btnCenter = new JButton("start ALL ONLINE");

  // Menu
  private JMenuBar mb = new JMenuBar(); // Menubar
  private JMenu mnuFile = new JMenu("File"); // File Entry on Menu bar
  private JMenuItem mnuItemQuit = new JMenuItem("Quit"); // Quit sub item
  private JMenu mnuHelp = new JMenu("Help"); // Help Menu entry
  private JMenuItem mnuItemAbout = new JMenuItem("About"); // About Entry

  public static String OVCC_IP;
  public static int OVCC_Port;

  public static Socket clientSocket = null;


  /** Constructor for the GUI */
  public TCPClientGUI(){
    // Set menubar
    f.setJMenuBar(mb);

    //Build Menus
    mnuFile.add(mnuItemQuit);  // Create Quit line
    mnuHelp.add(mnuItemAbout); // Create About line
    mb.add(mnuFile);        // Add Menu items to form
    mb.add(mnuHelp);

    // Add Buttons
    pnlNorth.add(btnNorth);
    pnlSouth.add(btnSouth);
    pnlEast.add(btnEast);
    pnlWest.add(btnWest);
    pnlCenter.add(btnCenter);

    // Setup Main Frame
    f.getContentPane().setLayout(new BorderLayout());
    f.getContentPane().add(pnlNorth, BorderLayout.NORTH);
    f.getContentPane().add(pnlSouth, BorderLayout.SOUTH);
    f.getContentPane().add(pnlEast, BorderLayout.EAST);
    f.getContentPane().add(pnlWest, BorderLayout.WEST);
    f.getContentPane().add(pnlCenter, BorderLayout.CENTER);

    // Allows the Swing App to be closed
    f.addWindowListener(new ListenCloseWdw());

    //Add Menu listener
    mnuItemQuit.addActionListener(new ListenMenuQuit());
    btnCenter.addActionListener(new ListenButtonCenter());
    btnSouth.addActionListener(new ListenButtonSouth());
    btnNorth.addActionListener(new ListenButtonNorth());
    btnEast.addActionListener(new ListenButtonEast());
    btnWest.addActionListener(new ListenButtonWest());
  }

  /** TPC IP Client Connection to Server **/
  public void open_socket(){
    try{
      clientSocket = new Socket(OVCC_IP, OVCC_Port);
    }
    catch (Exception err) {
      err.printStackTrace();
    }
  }

  public void close_socket(Socket clientSocket){
    try{
      clientSocket.close();
    }
    catch (Exception err) {
      err.printStackTrace();
    }
  }

  /**Execute a script from local machine**/
  public void execute_startupscript(String Script){
    try{
      String script_location = System.getenv("DCONLINE_PATH") + "/DCOV/readout/startupscript/" + Script;
      Process p=Runtime.getRuntime().exec(script_location);
      p.waitFor();
      BufferedReader reader=new BufferedReader(new InputStreamReader(p.getInputStream()));
      String line=reader.readLine();
      while(line!=null)
      {
        System.out.println(line);
        line=reader.readLine();
      }

    }
    catch(Exception err) {
      err.printStackTrace();
    }
    System.out.printf("Script %s Executed\n",Script);
  }



  public void send_message(Socket clientSocket, String sentence){
    try{
      DataOutputStream outToServer = new DataOutputStream(clientSocket.getOutputStream());
      BufferedReader inFromServer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
      outToServer.writeBytes(sentence + '\n');
      //String ServerCommand = inFromServer.readLine();
      System.out.println(inFromServer.readLine());
    }
    catch (Exception err) {
      err.printStackTrace();
    }
  }


  public class ListenMenuQuit implements ActionListener{
    public void actionPerformed(ActionEvent e){
      close_socket(clientSocket);
      System.exit(0);
    }
  }

  public class ListenCloseWdw extends WindowAdapter{
    public void windowClosing(WindowEvent e){
      close_socket(clientSocket);
      System.exit(0);
    }
  }

  //Center Button - start online
  public class ListenButtonCenter implements ActionListener{
    public void actionPerformed(ActionEvent e){
      send_message(clientSocket,"startALLONLINE");
      execute_startupscript("startOV_GUI.sh");
    }
  }

  //North Button - startOVRC/restartOVRC
  public class ListenButtonNorth implements ActionListener{
    public void actionPerformed(ActionEvent e){
      send_message(clientSocket,"startOVRC");
    }
  }

  //South Button - start OV Calib
  public class ListenButtonSouth implements ActionListener{
    public void actionPerformed(ActionEvent e){
      send_message(clientSocket,"startOVCalib");
    }
  }

  //East Button - KILL ALL
  public class ListenButtonEast implements ActionListener{
    public void actionPerformed(ActionEvent e){
      send_message(clientSocket,"killALL");
    }
  }

  //West Button - Kill OV Calib
  public class ListenButtonWest implements ActionListener{
    public void actionPerformed(ActionEvent e){
      send_message(clientSocket,"killOVCalib");
    }
  }

  public void launchFrame(){
    // Display Frame
    f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
    f.pack(); //Adjusts panel to components for display
    f.setVisible(true);
  }

  public static void main(String argv[]){
    String detector;
    try {
      detector = System.getenv("DC_DET");
    } catch (Exception e) {
      System.out.println("[DEBUG] : failed to get DC_DET.");
      detector = "";
    }
    if (argv.length > 0) detector = argv[0];
    if (detector != null && detector.length() != 0) {
      detector = "_" + detector;
    } else {
      detector = "";
    }
    //read from config file
    try{
      String filelocation = System.getenv("DCONLINE_PATH") + "/config/";
      FileInputStream fstream = new FileInputStream(filelocation+"DCSpaceIP"+detector+".config");
      // Get the object of DataInputStream
      DataInputStream in = new DataInputStream(fstream);
      BufferedReader br = new BufferedReader(new InputStreamReader(in));
      String strLine;
      //Read File Line By Line
      while ((strLine = br.readLine()) != null)   {
        // Print the content on the console
        if(strLine.charAt(0) != '#') {
          String delims = "[:]";
          String searchfor = "DCOVCC_IP";
          String[] tokens = strLine.split(delims);
          for (int i = 0; i < tokens.length; i++){
            //int search = tokens[i].indexOf(searchfor);
            if(tokens[i].indexOf(searchfor) != -1){
              TCPClientGUI.OVCC_IP = tokens[i+1];
              System.out.printf("OVCC HOST: %s\n",TCPClientGUI.OVCC_IP);
            }
          }
          //System.out.println (strLine);
        }
      }
      //Close the input stream
      in.close();

      FileInputStream fstreamport = new FileInputStream(filelocation+"DCSpacePort"+detector+".config");
      // Get the object of DataInputStream
      DataInputStream inport = new DataInputStream(fstreamport);
      BufferedReader brport = new BufferedReader(new InputStreamReader(inport));
      //Read File Line By Line
      while ((strLine = brport.readLine()) != null)   {
        // Print the content on the console
        if(strLine.charAt(0) != '#') {
          String delims = "[ ]+";
          String searchfor = "DCOVCC_PORT";
          String[] tokens = strLine.split(delims);
          for (int i = 0; i < tokens.length; i++){
            //System.out.printf("OVCC PORT: %s\n",tokens[i]);
            if(tokens[i].indexOf(searchfor) != -1){
              TCPClientGUI.OVCC_Port = Integer.parseInt(tokens[i+1]);
              System.out.printf("OVCC PORT: %d\n",TCPClientGUI.OVCC_Port);
            }
          }
          //System.out.println (strLine);
        }
      }
      //Close the input stream
      in.close();
    }
    catch (Exception err) {
      err.printStackTrace();
    }
    //
    //
    //
    TCPClientGUI gui = new TCPClientGUI();
    gui.launchFrame();
    gui.open_socket();
  }
}

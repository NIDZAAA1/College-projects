
package student;

import java.sql.CallableStatement;
import java.sql.Connection;
import java.sql.Types;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import rs.etf.sab.operations.CourierRequestOperation;

/**
 *
 * @author nikol
 */
public class sn210229_CourierRequestOperation implements CourierRequestOperation{

    private static Connection conn = null;
    private static sn210229_CourierRequestOperation inst = null;
    private static List<Pair> zahtevi = null;
    private sn210229_CourierRequestOperation(){
        conn = Database.getInstance().getConnection();
        zahtevi = new ArrayList<>();
    }
    
    public static sn210229_CourierRequestOperation getInstance(){
        if (inst == null) inst = new sn210229_CourierRequestOperation();
        return inst;
    }
    
    

    @Override
    public boolean deleteCourierRequest(String username) { 
        Iterator<Pair> iterator = zahtevi.iterator();
        while (iterator.hasNext()) {
            Pair zahtev = iterator.next();
            if (zahtev.getFirst().equals(username)) {
                iterator.remove();
                return true;
            }
        }
        return false;
    }

    @Override
    public List<String> getAllCourierRequests() { 
        List<String> rez = new ArrayList<>();
        for(Pair zahtev : zahtevi){
            rez.add(zahtev.getFirst());
        }
        return rez;
    }

    @Override
    public boolean grantRequest(String username) {  
        String vehicle = "";
        for(Pair zahtev : zahtevi){
            if(zahtev.getFirst().equals(username)){
                vehicle = zahtev.getSecond();
            }
        }
        int rezultatIzvrsavanja = 0;
        try {
            String sql = "{call SP_Procedura(?, ?, ?)}";
            CallableStatement cs = conn.prepareCall(sql);

            cs.setString(1, username);
            cs.setString(2, vehicle);
            cs.registerOutParameter(3, Types.INTEGER);
            cs.execute();
            rezultatIzvrsavanja = cs.getInt(3);

        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
        if(rezultatIzvrsavanja == -1){
            return false;
        }
        return true;
    }
    
    @Override
    public boolean changeVehicleInCourierRequest(String username, String registrationNumber) { 
        for(Pair zahtev : zahtevi){
            if(zahtev.getFirst().equals(username)){
                zahtev.setSecond(registrationNumber);
                return true;
            }
        }
        return false;
    }
    
    @Override
    public boolean insertCourierRequest(String username, String registrationNumber) { 
        if(getAllCourierRequests().contains(username))return false;
        zahtevi.add(new Pair(username,registrationNumber));
        return true;
    }
}

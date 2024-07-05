
package student;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import rs.etf.sab.operations.GeneralOperations;

/**
 *
 * @author nikol
 */
public class sn210229_GeneralOperations implements GeneralOperations{

    
    private static Connection conn = null;
    private static sn210229_GeneralOperations inst = null;
    
    private sn210229_GeneralOperations(){
        conn = Database.getInstance().getConnection();
    }
    
    public static sn210229_GeneralOperations getInstance(){
        if (inst == null) inst = new sn210229_GeneralOperations();
        return inst;
    }
    
    @Override
    public void eraseAll() {
        String query = "delete from Prevozi where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Ponuda where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Paket where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Opstina where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Grad where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Kurir where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Administrator where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Korisnik where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
        
        query = "delete from Vozilo where 1 = 1";
        try {
            PreparedStatement ps = conn.prepareStatement(query,ResultSet.TYPE_FORWARD_ONLY, ResultSet.CONCUR_UPDATABLE);
            ps.executeUpdate();
        } catch (Exception e) {e.printStackTrace();}
    }
    
}

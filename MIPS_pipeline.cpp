

/*Akhil Wadhwa
   AW3509
   Final Code- LAB1
   MIPS 5 stage pipeline including branch instructions as well as hazards
*/



#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct { // Instruction Fetch Stage
    bitset<32>  PC;
    bool        nop;
};

struct IDStruct { //Instruction Decode Stage
    bitset<32>  Instr;
    bool        nop;
};

struct EXStruct {  //Execute Stage
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;//
    bitset<5>   Rs;//
    bitset<5>   Rt;//
    bitset<5>   Wrt_reg_addr;//
    bool        is_I_type;//
    bool        rd_mem;//
    bool        wrt_mem;//
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu
    bool        wrt_enable;//
    bool        nop;
};

struct MEMStruct { // Memory Stage
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem;
    bool        wrt_enable;
    bool        nop;
};

struct WBStruct { //Write back stage
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;
};

struct stateStruct { //A structure than has all the 5 stages
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public:
        bitset<32> Reg_data;
     	RF(){                             //Constructor
 			Registers.resize(32);
			Registers[0] = bitset<32> (0);
        }

        bitset<32> readRF(bitset<5> Reg_addr){ //Read the Register File
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }

        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data){ //Write to the Register File
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }

		void outputRF(){ //Ofstream->Output stream class to operate on files
			ofstream rfout;  //object for the ofstream class->rfout
			rfout.open("RFresult.txt",std::ios_base::app); //Create/Overwrite the file
			if (rfout.is_open()){ //If the file is open
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++){ 
					rfout << Registers[j]<<endl; //Print the contents of the 32 Registers in the file
				    }
                }
			else cout<<"Unable to open file";
			rfout.close(); //close the file
            }

	private:
		vector<bitset<32> >Registers; //Register is made as a private member of the class-> so that no one has direct access to the registers.
};

class INSMem{
	public:

        bitset<32> Instruction; //Instructions are public.
        INSMem(){ // Constructor for instrcution Memory
			IMem.resize(MemSize); // Resize the instruction Memory to have 1000 locations of 8 bits each.
            ifstream imem; //Make an object of the "Input File stream" class "ifstream"
			string line; // define a string 
			int i = 0;
			imem.open("imem.txt"); //open the Imem File
			if (imem.is_open()){ //If File is open 
                    while (getline(imem,line)){ //Get a line from the file
                        if(line.length()>8) //If the line is more than 8 characters
                            line=line.erase(8); //Erase the caracters
                        IMem[i] = bitset<8>(line); //Take the 8 bits and stor it line by line in the instruction memory
                        i++;
                        }
                    }
			else cout<<"Unable to open file";
			imem.close(); //Close the opened File
            }

		bitset<32> readInstr(bitset<32> ReadAddress) { //Function to read the instruction memory //ReadAddress=> current PC
			string insmem; //declare a string
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string()); 
                                                                        //Take the 4 consecutive PC loactions and form a single instruction of 32 bits.
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction; //Return the 32 bit-instruction
		}

    private:
        vector<bitset<8> > IMem; //Define a vector of a bitset containing 8 bits.
        //Instruction Memory is a private variable to the Instruction Memory class.
};

class DataMem{
    public:                     // All the implementations are same to the Instruction Memory 
        bitset<32> ReadData;
        DataMem(){

            DMem.resize(MemSize); //Declare the data Memory of 1000 locations with 8 bits in each location
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open()){

                while (getline(dmem,line)){

                    if(line.length()>8)
                        line=line.erase(8);

                    DMem[i] = bitset<8>(line);
                    i++;
                    }
			    }

            else cout<<"Unable to open file";
                dmem.close();
        }

        bitset<32> readDataMem(bitset<32> Address){
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;
            }

        void writeDataMem(bitset<32> Address, bitset<32> WriteData){ //Write the data to the memory-> 8 bits in 1 location. 4 times at a time.
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));
            }

        void outputDataMem(){ //Save the data memory in a file
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open()){
                for (int j = 0; j< 1000; j++){ //Write all the 1000 location on the file.
                    dmemout << DMem[j]<<endl;
                    }
                }   
            else 
                cout<<"Unable to open file";
            dmemout.close(); //Close the File
            }

    private:
		vector<bitset<8> > DMem; //Define a private member-> a vector of 8 bits
};

void printState(stateStruct state, int cycle){ //Prints the Values of the variables in different stages in a file

    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open()){

        printstate<<"State after executing cycle:\t"<<cycle<<endl;

        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl;

        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl;
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl;
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl;
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl;
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl;
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl;
    }
    else cout<<"Unable to open file";
    printstate.close();
}

unsigned long shiftbits(bitset<32> inst, int start){ //First Parameter--> 32 bit value/ Second parameter--> an integer specifying the no. of times you want to shift.

    unsigned long ulonginst;
    return ((inst.to_ulong())>>start);   //shifts the variable passed in as the first parameter to the right from the "start" bit.|| RETURNS the shifted value

}

bitset<32> signextend (bitset<16> Imm){//Extends the 16 bits value to 32 bits
    string sestring;
    if (Imm[15]==0){
        sestring = "0000000000000000"+Imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    else{
        sestring = "1111111111111111"+Imm.to_string<char,std::string::traits_type,std::string::allocator_type>();
    }
    return (bitset<32> (sestring));

}

bitset<32> BranchAddr (bitset<16> Immi){ //Make a 32 bit branch address using 16 bits
    string Addr;
    if (Immi[15]==0){
        Addr = "00000000000000"+Immi.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }
    else{
        Addr = "11111111111111"+Immi.to_string<char,std::string::traits_type,std::string::allocator_type>()+"00";
    }
    return (bitset<32> (Addr));

}

int main(){

    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    int cycle=0;

    bitset<6> opcode;
    bitset<6> funct;

    bool IsLoad;
    bool IsStore;
    bool IsBranch;

    int stall=0;
    int counter=2;
    int branch=0;
    //int b=2;

    stateStruct state, newState;//stateStruct objects 

    state.IF.nop=0;
    newState.IF.nop=0;

    state.ID.nop=1;
    newState.ID.nop=1;

    state.EX.nop=1;
    newState.EX.nop=1;

    state.MEM.nop=1;
    newState.MEM.nop=1;

    state.WB.nop=1;
    newState.WB.nop=1;



    while (1){

        /* --------------------- WB stage --------------------- */

        if(state.WB.nop!=1){

            if(state.WB.wrt_enable==1){


                if(state.WB.Wrt_reg_addr==state.EX.Rs)
                    state.EX.Read_data1=state.WB.Wrt_data; //MEM to EX Forwarding for Rs

                if(state.WB.Wrt_reg_addr==state.EX.Rt)
                    state.EX.Read_data2=state.WB.Wrt_data;  //MEM to EX Forwarding for Rt


                if(state.WB.Wrt_reg_addr==state.MEM.Rt)
                    state.MEM.Store_data=state.WB.Wrt_data; //MEM to MEM forwarding for Rt. This dependency for Rs is resolved my MEM-EX, which already exists

                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);   //writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) //Writes back the data to RF Register.

                newState.WB.nop=state.WB.nop;
                }
            }

        /* --------------------- MEM stage --------------------- */

        if(state.MEM.nop!=1){

            if(state.MEM.rd_mem==1){
                newState.WB.Wrt_data=myDataMem.readDataMem(state.MEM.ALUresult);   //bitset<32> readDataMem(bitset<32> Address)
                }

            else
                newState.WB.Wrt_data=state.MEM.ALUresult;

            if(state.MEM.wrt_mem==1){
                    myDataMem.writeDataMem(state.MEM.ALUresult,state.MEM.Store_data);  //writeDataMem(bitset<32> Address, bitset<32> WriteData)
                    }

            if(state.MEM.wrt_enable){

                if(state.MEM.Wrt_reg_addr==state.EX.Rs)
                    state.EX.Read_data1=state.MEM.ALUresult;//EX to EX Forwarding for Rs

                if(state.MEM.Wrt_reg_addr==state.EX.Rt)
                    state.EX.Read_data2=state.MEM.ALUresult;//EX to EX Forwarding for Rt
                }
            
            newState.WB.wrt_enable=state.MEM.wrt_enable;
            newState.WB.Rt=state.MEM.Rt;
            newState.WB.Rs=state.MEM.Rs;
            newState.WB.Wrt_reg_addr=state.MEM.Wrt_reg_addr;
            newState.MEM.nop=state.MEM.nop;

            }

        newState.WB.nop=state.MEM.nop;


        /* --------------------- EX stage --------------------- */


        if(state.EX.nop!=1){

            if(state.EX.is_I_type==1){
                    state.EX.Read_data2=(signextend (state.EX.Imm));
                    }

            if(state.EX.alu_op==1){
                    newState.MEM.ALUresult= bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong());//
                    }
            else{
                    newState.MEM.ALUresult=bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());//
                    }
            newState.MEM.wrt_enable=state.EX.wrt_enable;
            newState.MEM.Rt=state.EX.Rt;
            newState.MEM.Rs=state.EX.Rs;
            newState.MEM.Wrt_reg_addr=state.EX.Wrt_reg_addr;
            newState.MEM.rd_mem=state.EX.rd_mem;
            newState.MEM.wrt_mem=state.EX.wrt_mem;
            newState.MEM.Store_data= (myRF.readRF(state.EX.Rt));
            }
        newState.MEM.nop=state.EX.nop;


        /* --------------------- ID stage --------------------- */

        if(state.ID.nop!=1){

            opcode = bitset<6> (shiftbits(state.ID.Instr, 26)); //#
            funct = bitset<6> (shiftbits(state.ID.Instr, 0)); //#

            newState.EX.is_I_type = (opcode.to_ulong()!=0 && opcode.to_ulong()!=2)?1:0; //#

            IsLoad = (opcode.to_ulong()==35)?1:0;
            IsStore = (opcode.to_ulong()==43)?1:0;
            IsBranch= (opcode.to_ulong()==4)?1:0;

            newState.EX.wrt_enable = (IsStore ||IsBranch)?0:1; //#
            newState.EX.Rs = bitset<5> (shiftbits(state.ID.Instr, 21)); //#
            newState.EX.Rt = bitset<5> (shiftbits(state.ID.Instr, 16)); //#
            newState.EX.Wrt_reg_addr =  (newState.EX.is_I_type)?newState.EX.Rt:bitset<5> (shiftbits(state.ID.Instr, 11));
            newState.EX.Imm = bitset<16>(shiftbits(state.ID.Instr, 0));

            newState.EX.alu_op= (opcode.to_ulong() ==35 || opcode.to_ulong() ==43 || funct.to_ulong() == 33)?1:0;
            newState.EX.rd_mem = (IsLoad)?1:0;
            newState.EX.wrt_mem = (IsStore)?1:0;
            newState.EX.Read_data1=(myRF.readRF(newState.EX.Rs));
            newState.EX.Read_data2=(myRF.readRF(newState.EX.Rt));
            newState.ID.nop=state.ID.nop;

        if(IsBranch){
            if(newState.EX.Read_data1!=newState.EX.Read_data2){
                branch=1;
                }
            }
        if(newState.MEM.rd_mem==1 && newState.MEM.wrt_mem!=1 && opcode.to_ulong()==0){
            if(newState.MEM.Wrt_reg_addr==newState.EX.Rs || newState.MEM.Wrt_reg_addr==newState.EX.Rt) {
                stall=1;
                counter=counter-1;
                }
            }
        }
        newState.EX.nop=state.ID.nop;

        /* --------------------- IF stage --------------------- */

        if (stall==1 && counter!=0){
            newState.EX.nop=1;
            }

        if (newState.ID.Instr.to_string<char,std::string::traits_type,std::string::allocator_type>()=="11111111111111111111111111111111"){    //Check if Halt //#
            newState.IF.PC = state.IF.PC;  //No Pc updation
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            state.IF.nop=1;//nop for the If state
            newState.IF.nop=1;
            }

        else if(stall==1 && counter!=0){
            newState.IF=state.IF;
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC.to_ulong()-4);
            stall=0;
            }
        else if(branch==1){
            newState.IF.PC = state.IF.PC.to_ulong()+BranchAddr(newState.EX.Imm).to_ulong();
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);

        }
        else {
            newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
            newState.IF.PC = bitset<32> (state.IF.PC.to_ulong()+4);

            if (newState.ID.Instr.to_string<char,std::string::traits_type,std::string::allocator_type>()=="11111111111111111111111111111111"){
                newState.IF.PC =state.IF.PC;  //No Pc updation
                newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
                state.IF.nop=1;//nop for the If state
                newState.IF.nop=1;
                }
            }

        newState.ID.nop=newState.IF.nop;
        if(branch==1){
            newState.ID.nop=1;
        }
        branch=0;


        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...
        cycle = cycle+1;
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */
        }
        myRF.outputRF(); // dump RF;
        myDataMem.outputDataMem(); // dump data mem
        return 0;
    }

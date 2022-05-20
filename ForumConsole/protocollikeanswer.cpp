#include "protocollikeanswer.h"

ProtocolLikeAnswer::ProtocolLikeAnswer(QString Username,QString Text)
{
    this->Text=Text;
    this->Username=Username;
}

void ProtocolLikeAnswer::createMessage()
{
    //Text.resize(Text.size()-2);
    if(searchAnswerOrQuestion("Answers",Text))
    {
        QSqlQuery query,queryCheck;
        queryCheck.prepare("select AR.ReputationType from AnswersReputation AR join Answers A on AR.IdAnswer=A.IdAnswer join Credentials C on C.IdUser=AR.IdUser where C.Username='"+Username+"' and A.Text='"+Text+"'");
        queryCheck.exec();
        queryCheck.next();

        if(queryCheck.value(0).toString()=="Like")
        {
            Message="11|0";
        }
        else
        {
            if(queryCheck.value(0).toString()=="Dislike")
                {
                query.prepare("update Answers set Reputation+=2 where Text='"+Text+"'");
                query.exec();
                query.next();

                query.exec("declare @IdA int;declare @IdUser int;  select @IdA=IdAnswer from Answers where Text='"+Text+"';select @IdUser=IdUser from Credentials where Username='"+Username+"'; update AnswersReputation set ReputationType='Like' where IdAnswer=@IdA and IdUser=@IdUser");
                query.next();
                Message="11|1";
                }

            else
            {
                query.prepare("update Answers set Reputation+=1 where Text='"+Text+"'");
                query.exec();
                query.next();

                QSqlQuery insertQuery;
                insertQuery.prepare("insert into [FORUM].[dbo].[AnswersReputation](IdAnswer,IdUser,ReputationType) values (:IdAns,:IdUs,:Type)");

                QSqlQuery queryFind;
                queryFind.prepare("select IdUser from Credentials where Username='"+Username+"'");
                queryFind.exec();
                queryFind.next();

                int IdUser=queryFind.value(0).toInt();

                queryFind.prepare("select IdAnswer from Answers where Text='"+Text+"'");
                queryFind.exec();
                queryFind.next();

                int IdAns=queryFind.value(0).toInt();

                QString type="Like";

                insertQuery.bindValue(":IdUs",IdUser);
                insertQuery.bindValue(":IdAns",IdAns);
                insertQuery.bindValue(":Type",type);

                if(insertQuery.exec())
                    Message="11|1";
                else
                    Message="11|0";

            }
        }
    }
    else
        Message="11|0";

}
void ProtocolLikeAnswer::writeOnSocket(QTcpSocket* socket,DataBase db)
{
    db.getDataBase().open();
    this->createMessage();
    db.getDataBase().close();
    socket->write(Message);
}
